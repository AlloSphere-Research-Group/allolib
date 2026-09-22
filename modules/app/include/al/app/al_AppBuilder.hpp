#ifndef INCLUDE_AL_APPBUILDER_HPP
#define INCLUDE_AL_APPBUILDER_HPP

/**
 * @file al_AppBuilder.hpp
 * @brief Fluent app composition over Runtime (option A).
 *
 * Configure which Modules exist, attach callbacks, then start().
 * Does not replace App / DistributedApp — those remain the desktop /
 * distributed recipes. AppBuilder is the preferred path for new code.
 *
 * Uses Runtime-owned Main pump (graphics LoopMode::Runtime) and idle when
 * there is no Main module — no KeepAliveDomain.
 *
 * Window remains a surface created by the graphics module (still a subdomain
 * type until the Surface refactor).
 */

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "al/app/al_AudioDomain.hpp"
#include "al/app/al_OSCDomain.hpp"
#include "al/app/al_OpenGLGraphicsDomain.hpp"
#include "al/app/al_Runtime.hpp"
#include "al/app/al_SimulationDomain.hpp"

namespace al {

class AppBuilder {
public:
  AppBuilder() = default;

  // --- module selection -------------------------------------------------

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

  // --- callbacks (typed wiring; no RTTI) --------------------------------

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

  // --- run --------------------------------------------------------------

  /// Create modules, wire callbacks, block until quit.
  void start() {
    ensureModules();
    wireAndRun();
  }

  Runtime &runtime() { return mRuntime; }
  Runtime const &runtime() const { return mRuntime; }

  std::shared_ptr<OpenGLGraphicsDomain> graphicsDomain() { return mGraphics; }
  std::shared_ptr<AudioDomain> audioDomain() { return mAudio; }
  std::shared_ptr<OSCDomain> oscDomain() { return mOsc; }
  std::shared_ptr<GLFWOpenGLWindowDomain> windowDomain() { return mWindow; }

private:
  void ensureModules() {
    if (mWantOsc) {
      mOsc = mRuntime.newDomain<OSCDomain>();
      mOsc->configure(mOscPort);
    }
    if (mWantAudio) {
      mAudio = mRuntime.newDomain<GammaAudioDomain>();
      mAudio->configure(mAudioRate, mAudioBlock, mAudioOuts, mAudioIns);
    }
    if (mWantGraphics) {
      mGraphics = mRuntime.newDomain<OpenGLGraphicsDomain>();
      mGraphics->setLoopMode(OpenGLGraphicsDomain::LoopMode::Runtime);
      mGraphics->nextWindowProperties.title = mTitle;
      mGraphics->nextWindowProperties.dimensions =
          Window::Dim(50, 50, mWidth, mHeight);
      mSimulation = mGraphics->newSubDomain<SimulationDomain>(true);
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

  bool mWantGraphics{false};
  bool mWantAudio{false};
  bool mWantOsc{false};

  std::string mTitle{"AlloApp"};
  int mWidth{800};
  int mHeight{600};
  double mAudioRate{44100};
  int mAudioBlock{512};
  int mAudioOuts{2};
  int mAudioIns{0};
  uint16_t mOscPort{9010};

  std::function<void()> mOnCreate;
  std::function<void(double)> mOnAnimate;
  std::function<void(Graphics &)> mOnDraw;
  std::function<void(AudioIOData &)> mOnSound;
  std::function<void(osc::Message &)> mOnMessage;

  std::shared_ptr<OpenGLGraphicsDomain> mGraphics;
  std::shared_ptr<SimulationDomain> mSimulation;
  std::shared_ptr<AudioDomain> mAudio;
  std::shared_ptr<OSCDomain> mOsc;
  std::shared_ptr<GLFWOpenGLWindowDomain> mWindow;
};

} // namespace al

#endif // INCLUDE_AL_APPBUILDER_HPP
