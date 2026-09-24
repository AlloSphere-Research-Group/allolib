#ifndef INCLUDE_AL_APPBUILDER_HPP
#define INCLUDE_AL_APPBUILDER_HPP

/**
 * @file al_AppBuilder.hpp
 * @brief Fluent app composition over Runtime.
 *
 * Preferred path for new code (including distributed). Optional .distributed()
 * loads NodeConfiguration from TOML and wires SceneStateBlob sync as graphics
 * Stages via attachSceneStateSync (default backend: Cuttlebone).
 */

#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "al/app/al_AudioDomain.hpp"
#include "al/app/al_OSCDomain.hpp"
#include "al/app/al_OpenGLGraphicsDomain.hpp"
#include "al/app/al_Runtime.hpp"
#include "al/app/al_SimulationDomain.hpp"
#include "al/app/al_StateDistributionDomain.hpp"
#include "al/app/al_StateSync.hpp"
#include "al/io/al_File.hpp"
#include "al/io/al_Toml.hpp"
#include "al/scene/al_SceneStateBlob.hpp"
#include "al/sphere/al_SphereUtils.hpp"
#include "al/system/al_NodeConfiguration.hpp"

// When the app links al_statedistribution, pull in cuttlebone registration.
// (Static ctors in the .a are often dead-stripped otherwise.)
#ifdef AL_USE_CUTTLEBONE
#include "al_ext/statedistribution/al_CuttleboneStateSync.hpp"
#endif

namespace al {

class AppBuilder {
public:
  AppBuilder() = default;

  AppBuilder &graphics(std::string title = "AlloApp", int w = 800, int h = 600) {
    mWantGraphics = true;
    mTitle = std::move(title);
    mWidth = w;
    mHeight = h;
    return *this;
  }

  AppBuilder &audio(double rate = 44100, int block = 512, int outs = 2,
                    int ins = 0) {
    mWantAudio = true;
    mAudioRate = rate;
    mAudioBlock = block;
    mAudioOuts = outs;
    mAudioIns = ins;
    return *this;
  }

  AppBuilder &osc(uint16_t port = 9010) {
    mWantOsc = true;
    mOscPort = port;
    return *this;
  }

  /**
   * Enable cluster roles + SceneStateBlob sync Stages on graphics.
   * Reads distributed_app.toml when present. Optional roleOverride forces
   * setRole without hostname match (e.g. "desktop" / "replica" for laptop).
   *
   * Default transport is StateBackend::Cuttlebone (not OscBlob).
   */
  AppBuilder &distributed(std::string tomlPath = "distributed_app.toml",
                          std::string roleOverride = "") {
    mWantDistributed = true;
    mClusterToml = std::move(tomlPath);
    mRoleOverride = std::move(roleOverride);
    return *this;
  }

  /// Select state transport backend (default Cuttlebone when distributed).
  AppBuilder &stateSync(StateBackend backend) {
    mStateCfg.backend = backend;
    return *this;
  }

  AppBuilder &stateSync(StateSyncConfig cfg) {
    mStateCfg = std::move(cfg);
    return *this;
  }

  AppBuilder &statePort(uint16_t port) {
    mStateCfg.port = port;
    return *this;
  }

  AppBuilder &onCreate(std::function<void()> fn) {
    mOnCreate = std::move(fn);
    return *this;
  }
  AppBuilder &onAnimate(std::function<void(double)> fn) {
    mOnAnimate = std::move(fn);
    return *this;
  }
  AppBuilder &onDraw(std::function<void(Graphics &)> fn) {
    mOnDraw = std::move(fn);
    return *this;
  }
  AppBuilder &onSound(std::function<void(AudioIOData &)> fn) {
    mOnSound = std::move(fn);
    return *this;
  }
  AppBuilder &onMessage(std::function<void(osc::Message &)> fn) {
    mOnMessage = std::move(fn);
    return *this;
  }

  void start() {
    if (mWantDistributed) {
      configureCluster();
    }
    ensureModules();
    wireAndRun();
  }

  Runtime &runtime() { return mRuntime; }
  Runtime const &runtime() const { return mRuntime; }

  std::shared_ptr<OpenGLGraphicsDomain> graphicsDomain() { return mGraphics; }
  std::shared_ptr<AudioDomain> audioDomain() { return mAudio; }
  std::shared_ptr<OSCDomain> oscDomain() { return mOsc; }
  std::shared_ptr<GLFWOpenGLWindowDomain> windowDomain() { return mWindow; }

  NodeConfiguration &node() { return mNode; }
  NodeConfiguration const &node() const { return mNode; }
  bool isPrimary() const { return mNode.isPrimary(); }
  bool isDistributed() const { return mWantDistributed; }

  SceneStateBlob &stateBlob() {
    if (mStateDomain) {
      return mStateDomain->state();
    }
    return mLocalBlob;
  }

  std::shared_ptr<StateDistributionDomain<SceneStateBlob>> stateDomain() {
    return mStateDomain;
  }

  StateSyncConfig const &stateSyncConfig() const { return mStateCfg; }

private:
  void configureCluster() {
    if (!mRoleOverride.empty()) {
      mNode.setRole(mRoleOverride);
      mNode.rank = (mRoleOverride == "desktop" || mRoleOverride == "simulator" ||
                    mRoleOverride == "control")
                       ? 0
                       : 1;
      std::cout << "[AppBuilder] role override: " << mRoleOverride
                << " rank=" << mNode.rank << "\n";
    }

    if (File::exists(mClusterToml)) {
      TomlLoader appConfig(mClusterToml);
      if (auto addr = appConfig.root->get_as<std::string>("broadcastAddress")) {
        mStateCfg.address = *addr;
      }
      auto nodesTable = appConfig.root->get_table_array("node");
      if (nodesTable && mRoleOverride.empty()) {
        const std::string host = al_get_hostname();
        for (const auto &table : *nodesTable) {
          std::string nodeHost = *table->get_as<std::string>("host");
          if (nodeHost != host) {
            continue;
          }
          if (auto role = table->get_as<std::string>("role")) {
            mNode.setRole(*role);
          }
          if (auto rank = table->get_as<int>("rank")) {
            mNode.rank = static_cast<uint16_t>(*rank);
          }
          std::cout << "[AppBuilder] cluster host=" << host
                    << " rank=" << mNode.rank << "\n";
          break;
        }
      }
    } else if (mRoleOverride.empty()) {
      mNode.setRole("desktop");
      mNode.rank = 0;
      std::cout << "[AppBuilder] no " << mClusterToml
                << " — default primary desktop\n";
    }
  }

  void ensureModules() {
    if (mWantOsc) {
      mOsc = mRuntime.newDomain<OSCDomain>();
      mOsc->configure(mOscPort);
    }
    if (mWantAudio &&
        (!mWantDistributed || mNode.hasCapability(CAP_AUDIO_IO))) {
      mAudio = mRuntime.newDomain<GammaAudioDomain>();
      mAudio->configure(mAudioRate, mAudioBlock, mAudioOuts, mAudioIns);
    }
    if (mWantGraphics) {
      mGraphics = mRuntime.newDomain<OpenGLGraphicsDomain>();
      mGraphics->setLoopMode(OpenGLGraphicsDomain::LoopMode::Runtime);
      mGraphics->nextWindowProperties.title = mTitle;
      mGraphics->nextWindowProperties.dimensions =
          Window::Dim(50, 50, mWidth, mHeight);
      if (mWantDistributed) {
        // Simulation Stage owns the POD slot; transport Stages attach later.
        mStateDomain =
            mGraphics->newSubDomain<StateDistributionDomain<SceneStateBlob>>(
                true);
        mSimulation = mStateDomain;
      } else {
        mSimulation = mGraphics->newSubDomain<SimulationDomain>(true);
      }
    }
  }

  void wireAndRun() {
    if (mGraphics && mOnCreate) {
      mGraphics->onCreate = mOnCreate;
    }
    if (mSimulation && mOnAnimate) {
      mSimulation->simulationFunction = mOnAnimate;
    }
    if (mAudio && mOnSound) {
      mAudio->onSound = mOnSound;
    }
    if (mOsc && mOnMessage) {
      mOsc->onMessage = mOnMessage;
    }

    mRuntime.initialize();

    if (mWantDistributed && mStateDomain) {
      const bool send =
          mNode.isPrimary() && mNode.hasCapability(CAP_STATE_SEND);
      const bool recv =
          !mNode.isPrimary() && mNode.hasCapability(CAP_STATE_RECEIVE);
      if (send || recv) {
#ifdef AL_USE_CUTTLEBONE
        if (mStateCfg.backend == StateBackend::Cuttlebone) {
          ensureCuttleboneSceneStateBackend();
        }
#endif
        if (!attachSceneStateSync(*mStateDomain, mStateCfg, send)) {
          std::cerr << "[AppBuilder] state sync attach failed ("
                    << StateSyncLimits::backendName(mStateCfg.backend)
                    << "). Distributed Scene sync disabled.\n";
        }
      }
    }

    if (mGraphics) {
      mWindow = mGraphics->newWindow();
      if (mOnDraw) {
        mWindow->onDraw = mOnDraw;
      }
    }

    mRuntime.run();
    mWindow.reset();
    mRuntime.cleanup();
  }

  Runtime mRuntime;
  NodeConfiguration mNode;
  SceneStateBlob mLocalBlob;
  StateSyncConfig mStateCfg; // defaults to Cuttlebone

  bool mWantGraphics{false};
  bool mWantAudio{false};
  bool mWantOsc{false};
  bool mWantDistributed{false};

  std::string mTitle{"AlloApp"};
  int mWidth{800};
  int mHeight{600};
  double mAudioRate{44100};
  int mAudioBlock{512};
  int mAudioOuts{2};
  int mAudioIns{0};
  uint16_t mOscPort{9010};
  std::string mClusterToml{"distributed_app.toml"};
  std::string mRoleOverride;

  std::function<void()> mOnCreate;
  std::function<void(double)> mOnAnimate;
  std::function<void(Graphics &)> mOnDraw;
  std::function<void(AudioIOData &)> mOnSound;
  std::function<void(osc::Message &)> mOnMessage;

  std::shared_ptr<OpenGLGraphicsDomain> mGraphics;
  std::shared_ptr<SimulationDomain> mSimulation;
  std::shared_ptr<StateDistributionDomain<SceneStateBlob>> mStateDomain;
  std::shared_ptr<AudioDomain> mAudio;
  std::shared_ptr<OSCDomain> mOsc;
  std::shared_ptr<GLFWOpenGLWindowDomain> mWindow;
};

} // namespace al

#endif // INCLUDE_AL_APPBUILDER_HPP
