#ifndef AL_APP_H
#define AL_APP_H

#include <cassert>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <stack>
#include <vector>

#include "al/app/al_AudioDomain.hpp"
#include "al/app/al_ComputationDomain.hpp"
#include "al/app/al_OSCDomain.hpp"
#include "al/app/al_OpenGLGraphicsDomain.hpp"
#include "al/app/al_SimulationDomain.hpp"
//#include "al_OpenVRDomain.hpp"

#include "al/app/al_WindowApp.hpp"

namespace al {

class App {
public:
  App();

  virtual ~App() {}

  template <class DomainType> std::shared_ptr<DomainType> newDomain() {
    auto newDomain = std::make_shared<DomainType>();
    mDomainList.push_back(newDomain);
    //    if (!newDomain->initialize()) {
    //      std::cerr << "ERROR initializing domain " << std::endl;
    //    }
    return newDomain;
  }

  virtual void onInit() {}
  virtual void onCreate() {}
  virtual void onAnimate(double dt) { (void)dt; }
  virtual void onDraw(Graphics &g) { (void)g; }
  virtual void onSound(AudioIOData &io) { (void)io; }
  virtual void onMessage(osc::Message &m) { (void)m; }
  virtual void onExit() {}

  virtual bool onKeyDown(Keyboard const &k) { return true; }
  virtual bool onKeyUp(Keyboard const &k) { return true; }
  virtual bool onMouseDown(Mouse const &m) { return true; }
  virtual bool onMouseUp(Mouse const &m) { return true; }
  virtual bool onMouseDrag(Mouse const &m) { return true; }
  virtual bool onMouseMove(Mouse const &m) { return true; }
  virtual bool onMouseScroll(Mouse const &m) { return true; }
  //  virtual void onResize(int w, int h) {}
  //  virtual void onVisibility(bool v) {}

  // Access to graphics domain and default window.

  Window &defaultWindow();
  Graphics &graphics();
  Keyboard &keyboard(); ///< Get current keyboard state
  Mouse &mouse();       ///< Get current mouse state

  double aspect(); ///< Get aspect ratio (width divided by height)
  bool created();  ///< Whether window has been created providing a valid
                   ///< graphics context
  Window::Cursor cursor();             ///< Get current cursor type
  bool cursorHide();                   ///< Whether the cursor is hidden
  Window::Dim dimensions();            ///< Get current dimensions of window
  Window::DisplayMode displayMode();   ///< Get current display mode
  bool enabled(Window::DisplayMode v); ///< Get whether display mode flag is set
  bool fullScreen();                   ///< Get whether window is in fullscreen
  const std::string &title();          ///< Get title of window
  bool visible();                      ///< Get whether window is visible
  bool vsync();                        ///< Get whether v-sync is enabled

  int height(); ///< Get window height, in pixels
  int width();  ///< Get window width, in pixels

  // get frambuffer size
  // it will be different from window widht and height
  // if the display is a high resolution one (ex: RETINA display)
  int fbHeight();
  int fbWidth();

  float highresFactor();

  bool decorated();

  void cursor(Window::Cursor v);               ///< Set cursor type
  void cursorHide(bool v);                     ///< Set cursor hiding
  void cursorHideToggle();                     ///< Toggle cursor hiding
  void dimensions(const Window::Dim &v);       ///< Set dimensions
  void dimensions(int w, int h);               ///< Set dimensions
  void dimensions(int x, int y, int w, int h); ///< Set dimensions
  void displayMode(Window::DisplayMode v); ///< Set display mode; will recreate
                                           ///< window if different from current

  /// This will make the window go fullscreen without borders and,
  /// if posssible, without changing the display resolution.
  void fullScreen(bool on);
  void fullScreenToggle(); ///< Toggle fullscreen
  void hide();             ///< Hide window (if showing)
  void iconify();          ///< Iconify window
  // void show(); ///< Show window (if hidden)
  void title(const std::string &v); ///< Set title
  void vsync(bool v); ///< Set whether to sync the frame rate to the monitor's
                      ///< refresh rate
  void decorated(bool b);

  Viewpoint &view();
  Pose &pose();
  Lens &lens();
  Nav &nav();
  NavInputControl &navControl();

  void quit();

  bool shouldQuit();

  void fps(double f);

  // Event handlers
  [[deprecated("Call through graphicsDomain()")]] void
  append(WindowEventHandler &handler);
  [[deprecated("Call through graphicsDomain()")]] void
  prepend(WindowEventHandler &handler);
  [[deprecated("Call through graphicsDomain()")]] void
  remove(WindowEventHandler &handler);

  // Access to audio domain
  [[deprecated("Use call from domain directly")]] AudioIO &audioIO();

  void configureAudio(double audioRate = 44100, int audioBlockSize = 512,
                      int audioOutputs = -1, int audioInputs = -1);

  void configureAudio(AudioDevice &dev, double audioRate, int audioBlockSize,
                      int audioOutputs, int audioInputs);

  // Access to OSC domain
  ParameterServer &parameterServer();

  virtual void start();

  // Domain access
  std::shared_ptr<OSCDomain> oscDomain() { return mOSCDomain; }
  std::shared_ptr<AudioDomain> audioDomain() { return mAudioDomain; }
  std::shared_ptr<OpenGLGraphicsDomain> graphicsDomain() {
    return mOpenGLGraphicsDomain;
  }
  std::shared_ptr<SimulationDomain> simulationDomain() {
    return mSimulationDomain;
  }

  struct StandardWindowAppKeyControls : WindowEventHandler {
    bool keyDown(const Keyboard &k) {
      if (k.ctrl()) {
        switch (k.key()) {
        case 'q':
          app->quit();
          return false;
          //          case 'h':
          //            window().hide();
          //            return false;
          //          case 'm':
          //            window().iconify();
          //            return false;
        case 'u':
          window().cursorHideToggle();
          return false;
          //          case 'w':
          //            app->graphicsDomain()->closeWindow(app->mDefaultWindowDomain);
          //            return false;
        default:;
        }
      } else {
        switch (k.key()) {
        case Keyboard::ESCAPE:
          window().fullScreenToggle();
          return false;
        default:;
        }
      }
      return true;
    }
    App *app;
  };
  StandardWindowAppKeyControls stdControls;

protected:
  void initializeDomains();

  std::shared_ptr<GLFWOpenGLWindowDomain> mDefaultWindowDomain;

  std::shared_ptr<OSCDomain> mOSCDomain;
  std::shared_ptr<AudioDomain> mAudioDomain;
  std::shared_ptr<OpenGLGraphicsDomain> mOpenGLGraphicsDomain;
  std::shared_ptr<SimulationDomain> mSimulationDomain;

  std::vector<std::shared_ptr<AsynchronousDomain>> mDomainList;
  std::stack<std::shared_ptr<AsynchronousDomain>> mRunningDomains;
};

class AudioControl {
public:
  void registerAudioIO(AudioIO &io) {
    gain.registerChangeCallback([&io](float value) { io.gain(value); });
  }

  Parameter gain{"gain", "sound", 1.0, "alloapp", 0.0, 2.0};
};

class DistributedApp : public App {
public:
  DistributedApp(bool primary = true) : App() {
    // State will be same memory for local, but will be synced on the network
    // for separate instances

    mOpenGLGraphicsDomain->removeSubDomain(simulationDomain());

    // Replace Simulation domain with state simulation domain
    mSimulationDomain =
        mOpenGLGraphicsDomain->newSubDomain<SimulationDomain>(true);
    mAudioControl.registerAudioIO(audioIO());
    parameterServer() << mAudioControl.gain;

    mPrimary = primary;
    if (mPrimary) {
      std::cout << "Running Primary" << std::endl;

    } else {
      std::cout << "Running REPLICA" << std::endl;

      mSimulationDomain
          ->disableProcessingCallback(); // Replicas won't call onAnimate()
    }
  }

private:
  bool mPrimary;
  AudioControl mAudioControl;
};

template <class TSharedState>
class DistributedAppWithState : public DistributedApp {
public:
  DistributedAppWithState(uint16_t rank_) : DistributedApp(), rank(rank_) {
    // State will be same memory for local, but will be synced on the network
    // for separate instances

    mOpenGLGraphicsDomain->removeSubDomain(simulationDomain());

    // Replace Simulation domain with state simulation domain
    mSimulationDomain =
        mOpenGLGraphicsDomain
            ->newSubDomain<StateSimulationDomain<TSharedState>>(true);

    if (rank == 0) {
      std::cout << "Running primary" << std::endl;
      auto sender =
          std::static_pointer_cast<StateSimulationDomain<TSharedState>>(
              mSimulationDomain)
              ->addStateSender("state");
      sender->configure(10101);
    } else {
      std::cout << "Running REPLICA" << std::endl;
      auto receiver =
          std::static_pointer_cast<StateSimulationDomain<TSharedState>>(
              mSimulationDomain)
              ->addStateReceiver("state");
      receiver->configure(10101);
      mSimulationDomain
          ->disableProcessingCallback(); // Replicas won't call onAnimate()
    }
  }

  TSharedState &state() {
    return std::static_pointer_cast<StateSimulationDomain<TSharedState>>(
               mSimulationDomain)
        ->state();
  }

  void setTitle(std::string title) { defaultWindow().title(title); }

  uint16_t rank{0};
  uint16_t group{0};

private:
};

} // namespace al

#endif // AL_APP_H
