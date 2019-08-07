#include <cstring>
#include <iostream>

#include "al/app/al_OpenGLGraphicsDomain.hpp"
#include "al/io/al_Window.hpp"

using namespace al;

bool OpenGLGraphicsDomain::initialize(ComputationDomain *parent) {
  bool ret = true;
  initializeWindowManager();
  callInitializeCallbacks();
  return ret;
}

bool OpenGLGraphicsDomain::start() {
  if (!mRunning) {
    mRunning = true;
    bool ret = true;
    ret &= initializeSubdomains(true);
    startFPS();  // WindowApp (FPS)
    ret &= initializeSubdomains(false);

    preOnCreate();
    onCreate();
    callStartCallbacks();
    while (!shouldQuit()) {
      mSubdomainLock.lock();
      tickSubdomains(true);
      tickFPS();
      tickSubdomains(false);
      mSubdomainLock.unlock();
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

  onExit();  // user defined
  postOnExit();

  ret &= cleanupSubdomains(false);
  mRunning = false;
  return ret;
}

bool OpenGLGraphicsDomain::cleanup(ComputationDomain *parent) {
  callCleanupCallbacks();
  terminateWindowManager();
  return true;
}

/// Window Domain ----------------------

GLFWOpenGLWindowDomain::GLFWOpenGLWindowDomain() {
  mWindow = std::make_unique<Window>();
  mGraphics = std::make_unique<Graphics>();
}

bool GLFWOpenGLWindowDomain::initialize(ComputationDomain *parent) {
  //  if (strcmp(typeid(*parent).name(), typeid(OpenGLGraphicsDomain).name()) ==
  //  0) {
  //    mGraphics = &static_cast<OpenGLGraphicsDomain *>(parent)->graphics();
  //  }
  assert(strcmp(typeid(*parent).name(), typeid(OpenGLGraphicsDomain).name()) ==
         0);
  mParent = static_cast<OpenGLGraphicsDomain *>(parent);
  if (!mWindow) {
    mWindow = std::make_unique<Window>();
  }
  if (!mGraphics) {
    mGraphics = std::make_unique<Graphics>();
  }
  if (!mWindow->created()) {
    bool ret = mWindow->create();
    mGraphics->init();
    return ret;
  }

  return true;
}

bool GLFWOpenGLWindowDomain::tick() {
  /* Make the window's context current */
  onNewFrame();
  mWindow->makeCurrent();
  preOnDraw();
  onDraw(*mGraphics);
  postOnDraw();
  mWindow->refresh();
  return true;
}

bool GLFWOpenGLWindowDomain::cleanup(ComputationDomain *parent) {
  if (mWindow) {
    mWindow->destroy();
    mWindow = nullptr;
  }
  if (mGraphics) {
    mGraphics = nullptr;
  }
  return true;
}
