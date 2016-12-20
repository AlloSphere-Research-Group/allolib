#include "al/core/app/al_WindowApp.hpp"

#include <algorithm> // max

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
  al_nsec dt = interval;
  nsec = 0;
  al_start_steady_clock();
  while (!should_quit && created()) {
    onAnimate(al_time_ns2s * dt);
    onDraw();
    refresh();
    al_nsec after_loop = al_steady_time_nsec();
    al_nsec to_sleep = std::max(interval - (after_loop - nsec), 0ll);
    al_sleep_nsec(to_sleep);
    dt = after_loop + to_sleep - nsec;
    nsec = after_loop + to_sleep;
  }
  onExit();
}

void al::WindowApp::quit() {
  should_quit = true;
}

void al::WindowApp::fps(double f) {
  _fps = f;
  interval = al_time_s2ns / f;
}

double al::WindowApp::fps() {
  return _fps;
}


double al::WindowApp::sec() {
  return nsec * al_time_ns2s;
}

double al::WindowApp::msec() {
  return nsec * 1.0e-6;
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