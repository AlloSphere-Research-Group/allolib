#pragma once

// SINGLE-WINDOW app

// makes window to be also an event handler
// by appending itself to the list of handlers

// also adds standard window controls, such as ctrl + q to quit

// Keehong Youn, 2016
// younkeehong@gmail.com

#include "al/core/app/al_Window.hpp"
#include "al/core/system/al_Time.hpp"
#include <atomic>

namespace al {

class WindowApp : public Window, public WindowEventHandler {
public:
  StandardWindowKeyControls stdControls;
  std::atomic<bool> should_quit;
  al_nsec interval = 16666666ll; // 60 fps
  al_nsec deltaTime;
  double mFPSWanted = 60;

  WindowApp();
  virtual ~WindowApp();

  void start(
    const Window::Dim& dim = Window::Dim(100, 100, 900, 450),
    const std::string title="AlloSystem",
    Window::DisplayMode mode = Window::DEFAULT_BUF
  );
  void quit();

  void fps(double f);
  double fpsWanted();
  double fps();

  // get time
  double sec();
  double msec(); // millis

  // place for app specific behind-the-scene loop logic (stereo, omni, default shader)
  void loop(double dt);
  
  // user override these
  virtual void onInit() {}
  virtual void onCreate() {}
  virtual void onAnimate(double dt) {}
  virtual void onDraw() {}
  virtual void onExit() {}
  virtual void onKeyDown(Keyboard const& k) {}
  virtual void onKeyUp(Keyboard const& k) {}
  virtual void onMouseDown(Mouse const& m) {}
  virtual void onMouseUp(Mouse const& m) {}
  virtual void onMouseDrag(Mouse const& m) {}
  virtual void onMouseMove(Mouse const& m) {}
  virtual void onResize(int w, int h) {}
  virtual void onVisibility(bool v) {}

  // call user event functions using WindowEventHandler class
  virtual bool keyDown(const Keyboard& k) override;
  virtual bool keyUp(const Keyboard& k) override;
  virtual bool mouseDown(const Mouse& m) override;
  virtual bool mouseDrag(const Mouse& m) override;
  virtual bool mouseMove(const Mouse& m) override;
  virtual bool mouseUp(const Mouse& m) override;
  virtual bool resize(int dw, int dh) override;
  virtual bool visibility(bool v) override;

};

}