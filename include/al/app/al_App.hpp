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
#include "al/graphics/al_Graphics.hpp"

/** @defgroup App Application building tools
 *
 */

namespace al {

/**
 * @brief Simple App class
 * @ingroup App
 */
class App {
public:
  App();

  virtual ~App() {}

  template <class DomainType> std::shared_ptr<DomainType> newDomain() {
    auto newDomain = std::make_shared<DomainType>();
    mDomainList.push_back(newDomain);
    //    if (!newDomain->init()) {
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

  virtual bool onKeyDown(Keyboard const & /*k*/) { return true; }
  virtual bool onKeyUp(Keyboard const & /*k*/) { return true; }
  virtual bool onMouseDown(Mouse const & /*m*/) { return true; }
  virtual bool onMouseUp(Mouse const & /*m*/) { return true; }
  virtual bool onMouseDrag(Mouse const & /*m*/) { return true; }
  virtual bool onMouseMove(Mouse const & /*m*/) { return true; }
  virtual bool onMouseScroll(Mouse const & /*m*/) { return true; }
  virtual void onResize(int /*w*/, int /*h*/) {}
  //  virtual void onVisibility(bool v) {}

  void quit(); ///< Requests domain to quit.
  bool shouldQuit();

  // Access to graphics domain properties
  virtual Window &defaultWindow();
  virtual Graphics &graphics();

  virtual Viewpoint &view();
  virtual Pose &pose();
  virtual Lens &lens();
  virtual Nav &nav();
  virtual NavInputControl &navControl();
  void fps(double f);

  // Access to default window domain
  // These functions are only valid after start() has been called.
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

  void fullScreenToggle(); ///< Toggle fullscreen
  void hide();             ///< Hide window (if showing)
  void iconify();          ///< Iconify window
  // void show(); ///< Show window (if hidden)

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
  void title(const std::string &v); ///< Set title
  void vsync(bool v); ///< Set whether to sync the frame rate to the monitor's
                      ///< refresh rate
  void decorated(bool b);

  // Event handlers
  [[deprecated("Call through defaultWindow()")]] void
  append(WindowEventHandler &handler);
  [[deprecated("Call through defaultWindow()")]] void
  prepend(WindowEventHandler &handler);
  [[deprecated("Call through defaultWindow()")]] void
  remove(WindowEventHandler &handler);

  // Access to audio domain
  AudioIO &audioIO();

  void configureAudio(double audioRate = 44100, int audioBlockSize = 512,
                      int audioOutputs = -1, int audioInputs = -1);

  void configureAudio(AudioDevice &dev, double audioRate = -1,
                      int audioBlockSize = 512, int audioOutputs = -1,
                      int audioInputs = -1);

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

  std::shared_ptr<GLFWOpenGLWindowDomain> defaultWindowDomain() {
    return mDefaultWindowDomain;
  }
  struct StandardWindowAppKeyControls : WindowEventHandler {
    bool keyDown(const Keyboard &k);
    App *app;
  };
  StandardWindowAppKeyControls stdControls;

  // Modify these with care
  std::shared_ptr<GLFWOpenGLWindowDomain> mDefaultWindowDomain;

  std::shared_ptr<OSCDomain> mOSCDomain;
  std::shared_ptr<AudioDomain> mAudioDomain;
  std::shared_ptr<OpenGLGraphicsDomain> mOpenGLGraphicsDomain;
  std::shared_ptr<SimulationDomain> mSimulationDomain;

protected:
  void createDomains();
  void initializeDomains();

  std::vector<std::shared_ptr<AsynchronousDomain>> mDomainList;
  std::stack<std::shared_ptr<AsynchronousDomain>> mRunningDomains;
};

} // namespace al

#endif // AL_APP_H
