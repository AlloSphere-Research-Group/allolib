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

#include <atomic>

namespace al {

class WindowApp : public Window, public WindowEventHandler, public FPS {
 public:
  // basic window app keyboard actions: fullscreen, quit, ...
  struct StandardWindowAppKeyControls : WindowEventHandler {
    bool keyDown(const Keyboard& k);
  };
  StandardWindowAppKeyControls stdControls;

  std::atomic<bool> mShouldQuitApp{false};
  bool is_verbose = false;
  void verbose(bool b = true) { is_verbose = b; }
  
  WindowApp();

  virtual void start();
  virtual void open();
  virtual void loop();
  virtual void closeApp();

  void quit() { mShouldQuitApp = true; }
  bool shouldQuit() { return mShouldQuitApp || Window::shouldClose(); }

  // user will override these
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

}  // namespace al

#endif