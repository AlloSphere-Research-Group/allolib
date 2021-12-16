#include "al/app/al_OpenGLGraphicsDomain.hpp"

#include <cstring>
#include <iostream>

#include "al/io/al_Window.hpp"

using namespace al;

bool OpenGLGraphicsDomain::init(ComputationDomain *parent) {
  bool ret = true;
  ret &= initializeSubdomains(true);
  initializeWindowManager();
  callInitializeCallbacks();
  ret &= initializeSubdomains(false);
  return ret;
}

bool OpenGLGraphicsDomain::start() {
  if (!mRunning) {
    mRunning = true;
    bool ret = true;
    ret &= initializeSubdomains(true);
    startFPS(); // WindowApp (FPS)
    ret &= initializeSubdomains(false);

    preOnCreate();
    onCreate();
    callStartCallbacks();
    bool subdomainsOk = true;
    while (!shouldQuit() && subdomainsOk) {
      subdomainsOk &= tickSubdomains(true);
      tickFPS();
      mTimeDrift = dt_sec();
      subdomainsOk &= tickSubdomains(false);
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

  ret &= cleanupSubdomains(false);
  mRunning = false;
  return ret;
}

bool OpenGLGraphicsDomain::cleanup(ComputationDomain *parent) {
  callCleanupCallbacks();
  terminateWindowManager();
  return true;
}

std::shared_ptr<GLFWOpenGLWindowDomain> OpenGLGraphicsDomain::newWindow() {
  auto newWindowDomain = newSubDomain<GLFWOpenGLWindowDomain>();
  newWindowDomain->init(this);
  newWindowDomain->window().decorated(nextWindowProperties.decorated);
  newWindowDomain->window().cursor(nextWindowProperties.cursor);
  newWindowDomain->window().cursorHide(!nextWindowProperties.cursorVisible);
  newWindowDomain->window().dimensions(nextWindowProperties.dimensions);
  newWindowDomain->window().displayMode(nextWindowProperties.displayMode);
  newWindowDomain->window().fullScreen(nextWindowProperties.fullScreen);
  newWindowDomain->window().title(nextWindowProperties.title);
  newWindowDomain->window().vsync(nextWindowProperties.vsync);

  return newWindowDomain;
}

void OpenGLGraphicsDomain::closeWindow(
    std::shared_ptr<GLFWOpenGLWindowDomain> windowDomain) {
  removeSubDomain(std::static_pointer_cast<SynchronousDomain>(windowDomain));
}

/// Window Domain ----------------------

GLFWOpenGLWindowDomain::GLFWOpenGLWindowDomain() {
  mWindow = std::make_unique<Window>();
  mGraphics = std::make_unique<Graphics>();
}

bool GLFWOpenGLWindowDomain::init(ComputationDomain *parent) {
  //  if (strcmp(typeid(*parent).name(), typeid(OpenGLGraphicsDomain).name()) ==
  //  0) {
  //    mGraphics = &static_cast<OpenGLGraphicsDomain *>(parent)->graphics();
  //  }
  bool ret = true;
  assert(strcmp(typeid(*parent).name(), typeid(OpenGLGraphicsDomain).name()) ==
         0);

  ret &= initializeSubdomains(true);
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
  ret &= initializeSubdomains(false);
  mInitialized = true;
  return ret;
}

bool GLFWOpenGLWindowDomain::tick() {
  if (mWindow->shouldClose()) {
    return false;
  }
  onNewFrame();
  /* Make the window's context current */
  mWindow->makeCurrent();
  tickSubdomains(true);
  preOnDraw();
  onDraw(*mGraphics);
  postOnDraw();
  tickSubdomains(false);
  mWindow->refresh();
  return true;
}

bool GLFWOpenGLWindowDomain::cleanup(ComputationDomain *parent) {
  bool ret = true;
  ret &= cleanupSubdomains(true);
  ret &= cleanupSubdomains(false);

  if (mWindow) {
    mWindow->destroy();
    mWindow = nullptr;
  }
  if (mGraphics) {
    mGraphics = nullptr;
  }
  mInitialized = false;
  return ret;
}
