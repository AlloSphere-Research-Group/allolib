#include "al/core/app/al_WindowApp.hpp"
#include "al/core/gl/al_GLFW.hpp"
#include "al/core/gl/al_GPUObject.hpp"

using namespace al;

// WindowApp::WindowApp(): mShouldQuitApp(false) {
//     append(stdControls);
//     append(windowEventHandler());
// }

// WindowApp::~WindowApp() {}

void WindowApp::open() {
  glfw::init();
  onInit();
  create();
  onCreate();
}

void WindowApp::loop() {
  // onAnimate(al_time_ns2s * deltaTime);
  onDraw();
  refresh();
}

void WindowApp::close() {
  // onExit();
  // free all GPUObjects (shaders, textures, vao, bo, ...)
  // before terminating glfw (closing window)
  GPUObject::destroyAll();
  destroy(); // destroy window
  glfw::terminate(); // this also closes existing windows
}

void WindowApp::start() {
  append(stdControls);
  append(windowEventHandler());
  open();
  startFPS();
  while (!shouldQuit()) {
    loop();
    tickFPS();
  }
  close();
}

void WindowApp::quit() {
  mShouldQuitApp = true;
}

bool WindowApp::shouldQuit() {
  return mShouldQuitApp || shouldClose();
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