#include <iostream>
#include <cstring>

#include "al_GraphicsDomain.hpp"

using namespace al;

bool GraphicsDomain::initialize(ComputationDomain *parent) {
  bool ret = true;
  glfw::init(app.is_verbose);
  ret &= glfwInit();

  if (app.is_verbose) std::cout << "Initialized GLFW " << glfwGetVersionString() << std::endl;
  glfwSetErrorCallback([](int code, const char* description){std::cout << "glfw error [" << code << "]: " << description << std::endl;});

  callInitializeCallbacks();
  return ret;
}

bool GraphicsDomain::start() {
  bool ret = true;
  ret &= initializeSubdomains(true);
  app.startFPS(); // WindowApp (FPS)
  gam::Domain::spu(app.fps());
  app.create(app.is_verbose);
  ret &= initializeSubdomains(false);

  preOnCreate();
  onCreate();
  callStartCallbacks();
  while (!app.shouldQuit()) {
    // to quit, call WindowApp::quit() or click close button of window,
    // or press ctrl + q
    preOnAnimate(app.dt_sec());
    onAnimate(app.dt_sec());
    mSubdomainLock.lock();
    tickSubdomains(true);
    preOnDraw();
    onDraw(app.mGraphics);
    postOnDraw();
    app.refresh();
    tickSubdomains(false);
    mSubdomainLock.unlock();
    app.tickFPS();
  }

  return ret;
}

bool GraphicsDomain::stop() {
  callStopCallbacks();
  onExit(); // user defined
  postOnExit();
  app.destroy();
  return true;
}

bool GraphicsDomain::cleanup(ComputationDomain *parent) {
  callCleanupCallbacks();
  glfw::terminate(app.is_verbose);
  return true;
}
