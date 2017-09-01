#ifndef INCLUDE_AL_WINDOWAPP_HPP
#define INCLUDE_AL_WINDOWAPP_HPP

// SINGLE-WINDOW app

// makes window to be also an event handler
// by appending itself to the list of handlers

// also adds standard window controls, such as ctrl + q to quit

// Keehong Youn, 2016
// younkeehong@gmail.com

#include "al/core/io/al_Window.hpp"
#include "al/core/app/al_FPS.hpp"

#include <atomic>

namespace al {

class WindowApp : public Window, public WindowEventHandler,
  public FPS
{
public:
  // basic window app keyboard actions: fullscreen, quit, ...
  struct StandardWindowAppKeyControls : WindowEventHandler {
    WindowApp* app;
    bool keyDown(const Keyboard& k);
  };
  StandardWindowAppKeyControls stdControls;
  std::atomic<bool> mShouldQuitApp {false};
  int mFrameCount = 0;

  WindowApp();
  // virtual ~WindowApp();

  void quit() {
    mShouldQuitApp = true;
  }
  bool shouldQuit() {
    return mShouldQuitApp || shouldClose();
  }

  // could and should be overrided
  // when subclass inherits from other app classes (AudioApp, etc.)
  virtual void start();

  // init glfw and open a window
  virtual void open();
  // app specific behind-the-scene loop logic
  // could add stereo, omni, default shader
  virtual void loop();
  // destroy gpu objects, close window, terminate glfw
  virtual void close();

  // user will override these
  virtual void onInit() {}
  virtual void onCreate() {}
  virtual void onDraw() {}
  virtual void onKeyDown(Keyboard const& k) {}
  virtual void onKeyUp(Keyboard const& k) {}
  virtual void onMouseDown(Mouse const& m) {}
  virtual void onMouseUp(Mouse const& m) {}
  virtual void onMouseDrag(Mouse const& m) {}
  virtual void onMouseMove(Mouse const& m) {}
  virtual void onResize(int w, int h) {}
  virtual void onVisibility(bool v) {}

  // for child classes to override
  virtual void preOnCreate() {}
  virtual void postOnCreate() {}
  virtual void preOnDraw() {}
  virtual void postOnDraw() {}

  // call user event functions using WindowEventHandler class
  virtual bool keyDown(const Keyboard& k) override;
  virtual bool keyUp(const Keyboard& k) override;
  virtual bool mouseDown(const Mouse& m) override;
  virtual bool mouseDrag(const Mouse& m) override;
  virtual bool mouseMove(const Mouse& m) override;
  virtual bool mouseUp(const Mouse& m) override;
  virtual bool resize(int dw, int dh) override;
  virtual bool visibility(bool v) override;

  int frameCount() {
    return mFrameCount;
  }
};

}

#endif