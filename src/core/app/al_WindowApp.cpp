#include "al/core/app/al_WindowApp.hpp"
#include "al/core/gl/al_GLFW.hpp"
#include "al/core/gl/al_GPUObject.hpp"

#include <algorithm> // max

using namespace al;

WindowApp::WindowApp(): should_quit(false) {
    append(stdControls);
    append(windowEventHandler());
}

WindowApp::~WindowApp() {}

void WindowApp::loop(double dt) {
    onAnimate(al_time_ns2s * deltaTime);
    onDraw();
}

void WindowApp::start() {
  glfw::init();
  onInit();
  create();
  onCreate();
  deltaTime = interval;
  al_nsec before_loop = 0;
  al_start_steady_clock();
  while (!should_quit && !shouldClose()) {
    loop(deltaTime);
    refresh();
    al_nsec after_loop = al_steady_time_nsec();
    al_nsec to_sleep = std::max(interval - (after_loop - before_loop), 0ll);
    al_sleep_nsec(to_sleep);
    deltaTime = (after_loop + to_sleep) - before_loop; // new before loop - old before loop
    before_loop = after_loop + to_sleep;
  }
  onExit();
  // free all GPUObjects (shaders, textures, vao, bo, ...) before terminating glfw (closing window)
  GPUObject::destroyAll();
  glfw::destroy(); // this also closes existing windows
}

void WindowApp::quit() {
  should_quit = true;
}

void WindowApp::fps(double f) {
  mFPSWanted = f;
  interval = al_time_s2ns / f;
}

double WindowApp::fpsWanted() {
  return mFPSWanted;
}

double WindowApp::fps() {
    return al_time_s2ns / deltaTime;
}

double WindowApp::sec() {
  return  al_steady_time_nsec() * al_time_ns2s;
}

double WindowApp::msec() {
  return  al_steady_time_nsec() * 1.0e-6;
}


// call user event functions using WindowEventHandler class
bool WindowApp::keyDown(const Keyboard& k) {
  onKeyDown(k);
  return true;
}

bool WindowApp::keyUp(const Keyboard& k) {
  onKeyUp(k);
  return true;
}

bool WindowApp::mouseDown(const Mouse& m) {
  onMouseDown(m);
  return true;
}

bool WindowApp::mouseDrag(const Mouse& m) {
  onMouseDrag(m);
  return true;
}

bool WindowApp::mouseMove(const Mouse& m) {
  onMouseMove(m);
  return true;
}

bool WindowApp::mouseUp(const Mouse& m) {
  onMouseUp(m);
  return true;
}

bool WindowApp::resize(int dw, int dh) {
  onResize(dw, dh);
  return true;
}

bool WindowApp::visibility(bool v) {
  onVisibility(v);
  return true;
}