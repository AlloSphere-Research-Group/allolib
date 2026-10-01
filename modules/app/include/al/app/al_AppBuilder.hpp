#ifndef INCLUDE_AL_APPBUILDER_HPP
#define INCLUDE_AL_APPBUILDER_HPP

/**
 * @file al_AppBuilder.hpp
 * @brief Fluent app composition over Runtime.
 *
 * Preferred path for new code (including distributed). Optional .distributed()
 * loads NodeConfiguration from TOML and wires SceneStateBlob sync as graphics
 * Stages via attachSceneStateSync (default backend: Cuttlebone).
 *
 * Surfaces:
 *   - CAP_OMNIRENDERING → GLFWOpenGLOmniRendererDomain (sphere / replica)
 *   - CAP_RENDERING     → flat GLFWOpenGLWindowDomain (desktop primary)
 */

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "al/app/al_AudioDomain.hpp"
#include "al/app/al_OSCDomain.hpp"
#include "al/app/al_OmniRendererDomain.hpp"
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

  /// Prefer output device whose name contains \p nameKeyword (e.g. "16A").
  /// Empty = system default. Call after / with .audio().
  AppBuilder &audioDevice(std::string nameKeyword) {
    mAudioOutDevice = std::move(nameKeyword);
    return *this;
  }

  /// Clamp requested I/O to what the selected device actually supports.
  /// Returns effective output channel count after clamp.
  static int clampChannelsToDevice(AudioDevice &dev, int requestedOuts,
                                   int requestedIns, int &outsOut,
                                   int &insOut) {
    const int maxOut = std::max(0, dev.channelsOutMax());
    const int maxIn = std::max(0, dev.channelsInMax());
    outsOut = requestedOuts;
    insOut = requestedIns;
    if (maxOut > 0 && outsOut > maxOut) {
      std::cerr << "[AppBuilder] WARNING: requested " << outsOut
                << " outs but \"" << dev.name() << "\" max is " << maxOut
                << " — clamping\n";
      outsOut = maxOut;
    }
    if (outsOut < 1 && maxOut > 0) {
      outsOut = std::min(2, maxOut);
    }
    if (maxIn > 0 && insOut > maxIn) {
      std::cerr << "[AppBuilder] WARNING: requested " << insOut
                << " ins but \"" << dev.name() << "\" max is " << maxIn
                << " — clamping\n";
      insOut = maxIn;
    }
    if (insOut < 0) {
      insOut = 0;
    }
    return outsOut;
  }

  AppBuilder &osc(uint16_t port = 9010) {
    mWantOsc = true;
    mOscPort = port;
    return *this;
  }

  /**
   * Enable cluster roles + SceneStateBlob sync Stages on graphics.
   * Reads distributed_app.toml when present. Optional roleOverride forces
   * setRole without hostname match (e.g. "desktop" / "replica").
   *
   * Roles with CAP_OMNIRENDERING (renderer / replica) get an omni Surface;
   * desktop / control keep a flat window.
   */
  AppBuilder &distributed(std::string tomlPath = "distributed_app.toml",
                          std::string roleOverride = "") {
    mWantDistributed = true;
    mClusterToml = std::move(tomlPath);
    mRoleOverride = std::move(roleOverride);
    return *this;
  }

  /// Force omni Surface even without CAP_OMNIRENDERING (desktop omni debug).
  AppBuilder &omni(bool enable = true) {
    mForceOmni = enable;
    return *this;
  }

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
  std::shared_ptr<GLFWOpenGLOmniRendererDomain> omniDomain() { return mOmni; }

  bool usesOmni() const { return static_cast<bool>(mOmni); }

  NodeConfiguration &node() { return mNode; }
  NodeConfiguration const &node() const { return mNode; }
  bool isPrimary() const { return mNode.isPrimary(); }
  bool isDistributed() const { return mWantDistributed; }

  /// Effective graphics-loop FPS (after first frames).
  double fps() const {
    return mGraphics ? mGraphics->fps() : 0.0;
  }

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
  bool shouldUseOmni() const {
    if (mForceOmni) {
      return true;
    }
    return mWantDistributed && mNode.hasCapability(CAP_OMNIRENDERING);
  }

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

    bool hostMatched = false;
    if (File::exists(mClusterToml)) {
      TomlLoader appConfig(mClusterToml);
      if (auto addr = appConfig.root->get_as<std::string>("broadcastAddress")) {
        mStateCfg.address = *addr;
      }
      // Optional [audio] table — device / channels / rate / buffer
      if (auto audioTable = appConfig.root->get_table("audio")) {
        mWantAudio = true;
        if (auto d = audioTable->get_as<std::string>("device")) {
          mAudioOutDevice = *d;
        }
        if (auto o = audioTable->get_as<int64_t>("channelsOut")) {
          mAudioOuts = static_cast<int>(*o);
        } else if (auto o32 = audioTable->get_as<int>("channelsOut")) {
          mAudioOuts = *o32;
        }
        if (auto i = audioTable->get_as<int64_t>("channelsIn")) {
          mAudioIns = static_cast<int>(*i);
        } else if (auto i32 = audioTable->get_as<int>("channelsIn")) {
          mAudioIns = *i32;
        }
        if (auto sr = audioTable->get_as<int64_t>("sampleRate")) {
          mAudioRate = static_cast<double>(*sr);
          mAudioRateLocked = true;
        } else if (auto srD = audioTable->get_as<double>("sampleRate")) {
          mAudioRate = *srD;
          mAudioRateLocked = true;
        }
        if (auto bs = audioTable->get_as<int64_t>("bufferSize")) {
          mAudioBlock = static_cast<int>(*bs);
        } else if (auto bs32 = audioTable->get_as<int>("bufferSize")) {
          mAudioBlock = *bs32;
        }
        std::cout << "[AppBuilder] [audio] from " << mClusterToml
                  << " device=\"" << mAudioOutDevice << "\" outs=" << mAudioOuts
                  << " ins=" << mAudioIns << " sr=" << mAudioRate
                  << " buf=" << mAudioBlock << "\n";
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
          hostMatched = true;
          std::cout << "[AppBuilder] cluster host=" << host
                    << " rank=" << mNode.rank << "\n";
          break;
        }
        if (!hostMatched) {
          std::cout << "[AppBuilder] host=" << host
                    << " not in " << mClusterToml
                    << " — defaulting to desktop (audio+graphics)\n";
        }
      }
    } else if (mRoleOverride.empty()) {
      std::cout << "[AppBuilder] no " << mClusterToml
                << " — default primary desktop\n";
    }

    // Env overrides (dev / one-off) — after TOML
    if (const char *e = std::getenv("AL_AUDIO_DEVICE")) {
      mAudioOutDevice = e;
    }
    if (const char *e = std::getenv("AL_AUDIO_OUTS")) {
      mAudioOuts = std::max(1, std::atoi(e));
    }
    if (const char *e = std::getenv("AL_AUDIO_INS")) {
      mAudioIns = std::max(0, std::atoi(e));
    }
    if (const char *e = std::getenv("AL_AUDIO_RATE")) {
      mAudioRate = std::atof(e);
      mAudioRateLocked = true;
    }
    if (const char *e = std::getenv("AL_AUDIO_BUF")) {
      mAudioBlock = std::max(16, std::atoi(e));
    }

    // Without a resolved role, capabilities stay CAP_NONE and ensureModules()
    // skips audio (and omni). Always land on desktop for local/dev hosts.
    if (!mNode.hasCapability(CAP_SIMULATOR) &&
        !mNode.hasCapability(CAP_RENDERING) &&
        !mNode.hasCapability(CAP_OMNIRENDERING) &&
        !mNode.hasCapability(CAP_AUDIO_IO)) {
      mNode.setRole("desktop");
      mNode.rank = 0;
    }

    std::cout << "[AppBuilder] state sync address=" << mStateCfg.address
              << " backend=Cuttlebone"
              << " audio=" << (mNode.hasCapability(CAP_AUDIO_IO) ? "yes" : "NO")
              << " primary=" << (mNode.isPrimary() ? "yes" : "no") << "\n";
    if (mStateCfg.address == "127.0.0.1" || mStateCfg.address == "localhost") {
      std::cerr
          << "[AppBuilder] WARNING: broadcast is loopback — LAN replicas will "
             "not see state. Set broadcastAddress in "
          << mClusterToml
          << " (e.g. \"192.168.10.255\") and run from the directory that "
             "contains it.\n";
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
      AudioDevice outDev = AudioDevice::defaultOutput();
      bool named = !mAudioOutDevice.empty();
      if (named) {
        AudioDevice found(mAudioOutDevice, AudioDevice::OUTPUT);
        if (!found.valid() || !found.hasOutput()) {
          std::cerr << "[AppBuilder] WARNING: audio device \"" << mAudioOutDevice
                    << "\" not found — falling back to default output\n";
          AudioDevice::printAll();
          named = false;
        } else {
          outDev = found;
        }
      }

      int outs = mAudioOuts;
      int ins = mAudioIns;
      clampChannelsToDevice(outDev, mAudioOuts, mAudioIns, outs, ins);
      mAudioOuts = outs;
      mAudioIns = ins;

      // Headphones / BT often want 48k; Motu may want 44.1. Prefer device
      // default unless TOML/env locked the rate.
      double rate = mAudioRate;
      const double pref = outDev.defaultSampleRate();
      if (!mAudioRateLocked && pref > 0.0) {
        rate = pref;
      }
      mAudioRate = rate;

      // Output-only when no inputs: avoid AirPods HFP / duplex quirks.
      if (ins <= 0) {
        std::cout << "[AppBuilder] audio out=\"" << outDev.name() << "\""
                  << (named ? "" : " (default)") << " outs=" << outs
                  << " (max " << outDev.channelsOutMax() << ") sr=" << rate
                  << (mAudioRateLocked ? " (locked)" : " (device default)")
                  << " buf=" << mAudioBlock << "\n";
        mAudio->configure(outDev, rate, mAudioBlock, outs, 0);
      } else {
        std::cout << "[AppBuilder] audio duplex out=\"" << outDev.name()
                  << "\" outs=" << outs << " ins=" << ins << " sr=" << rate
                  << "\n";
        mAudio->configure(outDev, rate, mAudioBlock, outs, ins);
      }
    } else if (mWantAudio) {
      std::cerr << "[AppBuilder] WARNING: audio requested but role has no "
                   "CAP_AUDIO_IO — no AudioDomain (silent).\n";
    }
    if (mWantGraphics) {
      mGraphics = mRuntime.newDomain<OpenGLGraphicsDomain>();
      mGraphics->setLoopMode(OpenGLGraphicsDomain::LoopMode::Runtime);
      mGraphics->nextWindowProperties.title = mTitle;
      mGraphics->nextWindowProperties.dimensions =
          Window::Dim(50, 50, mWidth, mHeight);
      if (mWantDistributed) {
        mStateDomain =
            mGraphics->newSubDomain<StateDistributionDomain<SceneStateBlob>>(
                true);
        mSimulation = mStateDomain;
      } else {
        mSimulation = mGraphics->newSubDomain<SimulationDomain>(true);
      }
    }
  }

  void createSurface() {
    if (!mGraphics) {
      return;
    }
    if (shouldUseOmni()) {
      // newSubDomain() would init immediately — configure Window first.
      mOmni = std::make_shared<GLFWOpenGLOmniRendererDomain>();
      mOmni->window().title(mTitle);
      mOmni->window().dimensions(50, 50, mWidth, mHeight);
      if (!mOmni->init(mGraphics.get())) {
        std::cerr << "[AppBuilder] omni Surface init failed\n";
        mOmni.reset();
        return;
      }
      mGraphics->addSubDomain(mOmni, false);
      mOmni->drawOmni = true;
      if (mOnDraw) {
        mOmni->onDraw = mOnDraw;
      }
      std::cout << "[AppBuilder] Surface=omni"
                << " drawOmni=" << (mOmni->drawOmni ? "yes" : "no")
                << " sphereRenderer="
                << (sphere::isRendererMachine() ? "yes" : "no")
                << " stereo=" << (mOmni->render_stereo ? "yes" : "no")
                << "\n";
    } else {
      mWindow = mGraphics->newWindow();
      if (mOnDraw) {
        mWindow->onDraw = mOnDraw;
      }
      std::cout << "[AppBuilder] Surface=flat window\n";
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

    createSurface();

    mRuntime.run();
    mWindow.reset();
    mOmni.reset();
    mRuntime.cleanup();
  }

  Runtime mRuntime;
  NodeConfiguration mNode;
  SceneStateBlob mLocalBlob;
  StateSyncConfig mStateCfg;

  bool mWantGraphics{false};
  bool mWantAudio{false};
  bool mWantOsc{false};
  bool mWantDistributed{false};
  bool mForceOmni{false};

  std::string mTitle{"AlloApp"};
  int mWidth{800};
  int mHeight{600};
  double mAudioRate{44100};
  bool mAudioRateLocked{false}; ///< true if TOML/env set sampleRate
  int mAudioBlock{512};
  int mAudioOuts{2};
  int mAudioIns{0};
  std::string mAudioOutDevice; ///< empty = defaultOutput()
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
  std::shared_ptr<GLFWOpenGLOmniRendererDomain> mOmni;
};

} // namespace al

#endif // INCLUDE_AL_APPBUILDER_HPP
