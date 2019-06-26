#include <iostream>
#include <cstring>

#include "al_OpenGLGraphicsDomain.hpp"

using namespace al;

bool OpenGLGraphicsDomain::initialize(ComputationDomain *parent) {
  bool ret = true;
  glfw::init(app.is_verbose);
  ret &= glfwInit();

  if (app.is_verbose) std::cout << "Initialized GLFW " << glfwGetVersionString() << std::endl;
  glfwSetErrorCallback([](int code, const char* description){std::cout << "glfw error [" << code << "]: " << description << std::endl;});

  onInit();
  callInitializeCallbacks();
  return ret;
}

bool OpenGLGraphicsDomain::start() {
  if (!mRunning) {
    mRunning = true;
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
      onNewFrame();

      mSubdomainLock.lock();
      tickSubdomains(true);

      app.makeCurrent();

      preOnDraw();
      onDraw(app.mGraphics);
      postOnDraw();
      app.refresh();

      tickSubdomains(false);
      mSubdomainLock.unlock();
      app.tickFPS();
    }

    ret &= stop();
    return ret;
  } else {
    return true;
  }
}

bool OpenGLGraphicsDomain::stop() {

  bool ret = true;
  callStopCallbacks();


  ret &= cleanupSubdomains(true);

  onExit(); // user defined
  postOnExit();
  app.destroy();

  ret &= cleanupSubdomains(false);
  mRunning = false;
  return ret;
}

bool OpenGLGraphicsDomain::cleanup(ComputationDomain *parent) {
  callCleanupCallbacks();
  glfw::terminate(app.is_verbose);
  return true;
}

/// Window Domain

bool OpenGLWindowDomain::initialize(ComputationDomain *parent) {
  if (strcmp(typeid(*parent).name(), typeid(OpenGLGraphicsDomain).name()) == 0) {
    mGraphics = &static_cast<OpenGLGraphicsDomain *>(parent)->graphics();
  }

  return mWindow.create();
}

bool OpenGLWindowDomain::tick() {
  /* Make the window's context current */
  mWindow.makeCurrent();
  preOnDraw();
  onDraw(*mGraphics);
  postOnDraw();
  mWindow.refresh();
  return true;
}

bool OpenGLWindowDomain::cleanup(ComputationDomain *parent) {
  mWindow.destroy();
  return true;
}
