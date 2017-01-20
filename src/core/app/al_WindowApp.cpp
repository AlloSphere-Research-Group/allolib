#include "al/core/app/al_WindowApp.hpp"
#include "al/core/gl/al_GLFW.hpp"

#include <algorithm> // max

al::WindowApp::WindowApp() : should_quit(false) {
  append(stdControls);
  append(windowEventHandler());
}

al::WindowApp::~WindowApp() {}

void al::WindowApp::loop(double dt) {
    onAnimate(al_time_ns2s * deltaTime);
    onDraw();
}

void al::WindowApp::start(
  const Window::Dim& dim,
  const std::string title,
  Window::DisplayMode mode
) {
  mDim = dim;
  mTitle = title;
  mDisplayMode = mode;
  al::glfw::init();
  onInit();
  create();
  onCreate();
  deltaTime = interval;
  al_nsec before_loop = 0;
  al_start_steady_clock();
  while (!should_quit && created()) {
    loop(deltaTime);
    refresh();
    al_nsec after_loop = al_steady_time_nsec();
    al_nsec to_sleep = std::max(interval - (after_loop - before_loop), 0ll);
    al_sleep_nsec(to_sleep);
    deltaTime = (after_loop + to_sleep) - before_loop; // new before loop - old before loop
    before_loop = after_loop + to_sleep;
  }
  onExit();
  al::glfw::destroy();
}

void al::WindowApp::quit() {
  should_quit = true;
}

void al::WindowApp::fps(double f) {
  mFPSWanted = f;
  interval = al_time_s2ns / f;
}

double al::WindowApp::fpsWanted() {
  return mFPSWanted;
}

double al::WindowApp::fps() {
    return al_time_s2ns / deltaTime;
}

double al::WindowApp::sec() {
  return  al_steady_time_nsec() * al_time_ns2s;
}

double al::WindowApp::msec() {
  return  al_steady_time_nsec() * 1.0e-6;
}


// call user event functions using WindowEventHandler class
bool al::WindowApp::keyDown(const Keyboard& k) {
  onKeyDown(k);
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