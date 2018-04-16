#ifndef INCLUDE_AL_WINDOWAPP_HPP
#define INCLUDE_AL_WINDOWAPP_HPP

// SINGLE-WINDOW app

// makes window to be also an event handler
// by appending itself to the list of handlers

// also adds standard window controls, such as ctrl + q to quit

// Keehong Youn, 2016
// younkeehong@gmail.com

#include "al/core/app/al_FPS.hpp"
#include "al/core/io/al_Window.hpp"
#include "al/core/graphics/al_GLFW.hpp"
#include "al/core/graphics/al_Graphics.hpp"

#include <atomic>

namespace al {

class WindowApp : public Window, public WindowEventHandler, public FPS {
 public:
  // basic window app keyboard actions: fullscreen, quit, ...
  struct StandardWindowAppKeyControls : WindowEventHandler {
    bool keyDown(const Keyboard& k);
  };
  StandardWindowAppKeyControls stdControls;

  Graphics mGraphics;
  std::atomic<bool> mShouldQuitApp{false};
  bool is_verbose = false;
  void verbose(bool b = true) { is_verbose = b; }
  
  WindowApp();

  // start app loop
  virtual void start();

  void quit() { mShouldQuitApp = true; }
  bool shouldQuit() { return mShouldQuitApp || Window::shouldClose(); }

  // user will override these
  virtual void onInit() {}
  virtual void onCreate() {}
  virtual void onAnimate(double dt) {}
  virtual void onDraw(Graphics& g) {}
  virtual void onExit() {}
  virtual void onKeyDown(Keyboard const& k) {}
  virtual void onKeyUp(Keyboard const& k) {}
  virtual void onMouseDown(Mouse const& m) {}
  virtual void onMouseUp(Mouse const& m) {}
  virtual void onMouseDrag(Mouse const& m) {}
  virtual void onMouseMove(Mouse const& m) {}
  virtual void onResize(int w, int h) {}
  virtual void onVisibility(bool v) {}

  // callbacks from window class, wil call user event functions like `on***`
  bool keyDown(const Keyboard& k) final;
  bool keyUp(const Keyboard& k) final;
  bool mouseDown(const Mouse& m) final;
  bool mouseDrag(const Mouse& m) final;
  bool mouseMove(const Mouse& m) final;
  bool mouseUp(const Mouse& m) final;
  bool resize(int dw, int dh) final;
  bool visibility(bool v) final;
};

}  // namespace al

#endif