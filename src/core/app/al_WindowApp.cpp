#include "al/core/app/al_WindowApp.hpp"
#include "al/core/gl/al_GLFW.hpp"
#include "al/core/gl/al_GPUObject.hpp"

using namespace al;

bool WindowApp::StandardWindowAppKeyControls::keyDown(const Keyboard& k){
  if(k.ctrl()){
    switch(k.key()){
      case 'q': app->quit(); return false;
      // case 'h': window().hide(); return false;
      // case 'm': window().iconify(); return false;
      case 'c': window().cursorHideToggle(); return false;
      default:;
    }
  }
  else{
    switch(k.key()){
      case Keyboard::ESCAPE: app->fullScreenToggle(); return false;
      default:;
    }
  }
  return true;
}

WindowApp::WindowApp() {
    stdControls.app = this;
    append(stdControls);
    append(windowEventHandler());
}

void WindowApp::open() {
  glfw::init();
  create();
  onCreate();
}

void WindowApp::loop() {
  onDraw();
  refresh();
  mFrameCount += 1;
}

void WindowApp::close() {
  destroy(); // destroy window
  glfw::terminate(); // this also closes existing windows
}

void WindowApp::start() {
  open();
  startFPS();
  while (!shouldQuit()) {
    loop();
    tickFPS();
  }
  close();
}

// call user event functions inside WindowEventHandler class
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