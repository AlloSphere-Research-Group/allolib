#include "al/core/app/al_WindowApp.hpp"

#include <thread>
#include <chrono>

al::WindowApp::WindowApp() : should_quit(false) {
  append(stdControls);
  append(windowEventHandler());
}
al::WindowApp::~WindowApp() {}

void al::WindowApp::start(
  const Window::Dim& dim,
  const std::string title,
  Window::DisplayMode mode
) {
  create(dim, title, mode);
  onCreate();
  while (!should_quit && created()) {
    onAnimate(0);
    onDraw();
    refresh();
    std::this_thread::sleep_for(chrono::milliseconds(100));
  }
  onExit();
}

void al::WindowApp::quit() {
  should_quit = true;
}

// call user event functions using WindowEventHandler class
bool al::WindowApp::keyDown(const Keyboard& k) {
  onKeydown(k);
  return true;
}

bool al::WindowApp::keyUp(const Keyboard& k) {
  onKeyUp(k);
  return true;
}

bool al::WindowApp::mouseDown(const Mouse& m) {
  onMouseDown(m);
  return true;
}

bool al::WindowApp::mouseDrag(const Mouse& m) {
  onMouseDrag(m);
  return true;
}

bool al::WindowApp::mouseMove(const Mouse& m) {
  onMouseMove(m);
  return true;
}

bool al::WindowApp::mouseUp(const Mouse& m) {
  onMouseUp(m);
  return true;
}

bool al::WindowApp::resize(int dw, int dh) {
  onResize(dw, dh);
  return true;
}

bool al::WindowApp::visibility(bool v) {
  onVisibility(v);
  return true;
}