#include "al/app/al_WindowApp.hpp"

using namespace al;

bool WindowApp::StandardWindowAppKeyControls::keyDown(const Keyboard& k) {
  if (k.ctrl()) {
    switch (k.key()) {
      case 'q':
        window().close();
        return false;
      // case 'h': window().hide(); return false;
      // case 'm': window().iconify(); return false;
      case 'u':
        window().cursorHideToggle();
        return false;
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

WindowApp::WindowApp() {
  append(stdControls);
  append(windowEventHandler());
}

void WindowApp::start() {
  initializeWindowManager();
  onInit();
  Window::create(is_verbose);
  onCreate();
  FPS::startFPS();
  while (!shouldQuit()) {
    onAnimate(dt_sec());
    onDraw(mGraphics);
    Window::refresh();
    FPS::tickFPS();
  }
  onExit();
  Window::destroy();          // destroy window
  terminateWindowManager();
}

bool WindowApp::resize(int dw, int dh) {
  onResize(dw, dh);
  return true;
}

bool WindowApp::visibility(bool v) {
  onVisibility(v);
  return true;
}
