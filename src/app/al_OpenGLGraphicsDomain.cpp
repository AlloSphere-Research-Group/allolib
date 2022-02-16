#include "al/app/al_OpenGLGraphicsDomain.hpp"

#include <cstring>
#include <iostream>

#include "al/io/al_Window.hpp"

using namespace al;

bool OpenGLGraphicsDomain::init(ComputationDomain *parent) {
  if (!mDomainThread) {
    mDomainStartSpinLock.store(true, std::memory_order_release);
    mDomainThread = std::make_unique<std::thread>(
        OpenGLGraphicsDomain::domainThreadFunction, this);
    // Wait for init to be done on thread;
    while (mDomainStartSpinLock.exchange(true, std::memory_order_acquire))
      ;
    return mCommandResult;
  }
  std::cout << "OpenGLGraphicsDomain already initialized" << std::endl;
  return true;
}

bool OpenGLGraphicsDomain::start() {
  if (!mRunning) {

    {
      std::lock_guard<std::mutex> lksig(mDomainSignalLock);
      mDomainCommand = CommandType::START;
    }
    std::unique_lock<std::mutex> lk(mDomainCommandLock);
    mDomainCommandSignal.wait(lk);
    return mCommandResult;
  }

  std::cout << "OpenGLGraphicsDomain already started" << std::endl;
  return true;
}

bool OpenGLGraphicsDomain::stop() {
  if (mRunning) {
    shouldStop();

    {
      std::lock_guard<std::mutex> lksig(mDomainSignalLock);
      mDomainCommand = CommandType::STOP;
    }
    std::unique_lock<std::mutex> lk(mDomainCommandLock);
    mDomainCommandSignal.wait(lk);

    return mCommandResult;
  }
  std::cout << "OpenGLGraphicsDomain not running" << std::endl;
  return true;
}

bool OpenGLGraphicsDomain::cleanup(ComputationDomain *parent) {
  if (mRunning) {
    stop();
  }
  if (mInitialized) {

    {
      std::lock_guard<std::mutex> lksig(mDomainSignalLock);
      mDomainCommand = CommandType::CLEANUP;
    }
    std::unique_lock<std::mutex> lk(mDomainCommandLock);
    mDomainCommandSignal.wait(lk);
    mDomainThread->join();

    return mCommandResult;
  }
  std::cout << "OpenGLGraphicsDomain not initialized" << std::endl;
  return true;
}

bool OpenGLGraphicsDomain::addSubDomain(
    std::shared_ptr<SynchronousDomain> subDomain, bool prepend) {
  if (!mRunning) {
    std::unique_lock<std::mutex> lk(mSubDomainInsertLock);
    mSubdomainToInsert = {subDomain, prepend};
    return true;
  } else {
    std::unique_lock<std::mutex> lk(mSubDomainInsertLock);
    mSubdomainToInsert = {subDomain, prepend};
    auto status = mSubDomainInsertSignal.wait_for(lk, std::chrono::seconds(10));
    // FIXME adjust for the case where the domain stops while doing this,
    // causing the wait to timeout
    if (status == std::cv_status::timeout) {
      std::cerr << "ERROR. Could not add sub domain" << std::endl;
      return false;
    }
  }
  return true;
}

bool OpenGLGraphicsDomain::removeSubDomain(
    std::shared_ptr<SynchronousDomain> subDomain) {
  if (!mRunning) {
    AsynchronousDomain::removeSubDomain(subDomain);
  } else {
    std::unique_lock<std::mutex> lk(mSubDomainRemoveLock);
    mSubdomainToRemove = subDomain;
    auto status = mSubDomainRemoveSignal.wait_for(lk, std::chrono::seconds(10));

    // FIXME adjust for the case where the domain stops while doing this,
    // causing the wait to timeout
    if (status == std::cv_status::timeout) {
      std::cerr << "ERROR. Could not remove sub domain" << std::endl;
      return false;
    }
  }
  return true;
}

std::shared_ptr<GLFWOpenGLWindowDomain> OpenGLGraphicsDomain::newWindow() {
  auto newWindowDomain = newSubDomain<GLFWOpenGLWindowDomain>();
  //  newWindowDomain->init(this);
  return newWindowDomain;
}

void OpenGLGraphicsDomain::closeWindow(
    std::shared_ptr<GLFWOpenGLWindowDomain> windowDomain) {
  removeSubDomain(std::static_pointer_cast<SynchronousDomain>(windowDomain));
}

void OpenGLGraphicsDomain::processDomainAddRemoveQueues(
    OpenGLGraphicsDomain *domain) {
  {
    std::unique_lock<std::mutex> lk(domain->mSubDomainInsertLock);
    if (domain->mSubdomainToInsert.first) {
      if (!domain->mSubdomainToInsert.first->init(domain)) {
        std::cerr << "ERROR initializing Window domain. Not injecting."
                  << std::endl;
      } else {
        domain->mSubDomainList.push_back(domain->mSubdomainToInsert);
      }

      domain->mSubDomainInsertSignal.notify_one();
      domain->mSubdomainToInsert.first = nullptr;
    }
  }
  {
    std::unique_lock<std::mutex> lk(domain->mSubDomainRemoveLock);
    if (domain->mSubdomainToRemove) {
      domain->removeSubDomain(domain->mSubdomainToRemove);

      domain->mSubDomainRemoveSignal.notify_one();
      domain->mSubdomainToRemove = nullptr;
    }
  }
}

void OpenGLGraphicsDomain::domainThreadFunction(OpenGLGraphicsDomain *domain) {
  domain->mCommandResult = domain->initPrivate();

  processDomainAddRemoveQueues(domain);
  domain->preOnCreate();
  domain->onCreate();
  domain->mDomainStartSpinLock.store(false, std::memory_order_release);
  while (domain->mInitialized) {
    bool subDomainsOk = true;
    // Tick the domain once then check for messages
    while (domain->mRunning && !domain->shouldStop() && subDomainsOk) {
      processDomainAddRemoveQueues(domain);
      subDomainsOk &= domain->tickSubdomains(true);
      subDomainsOk &= domain->tick();
      subDomainsOk &= domain->tickSubdomains(false);
      if (!subDomainsOk) {
        domain->shouldStop();
      }
    }

    {
      std::unique_lock<std::mutex> lk(domain->mDomainSignalLock);
      if (domain->mDomainCommand != CommandType::NONE) {

        if (domain->mDomainCommand == CommandType::START) {
          std::cout << "start message" << std::endl;
          domain->mCommandResult = domain->startPrivate();

        } else if (domain->mDomainCommand == CommandType::STOP) {
          domain->mCommandResult = domain->stopPrivate();

        } else if (domain->mDomainCommand == CommandType::CLEANUP) {
          domain->mCommandResult = domain->cleanupPrivate();
        }
        domain->mDomainCommandSignal.notify_one();
        domain->mDomainCommand = CommandType::NONE;
      }
    }
  }
}

bool OpenGLGraphicsDomain::initPrivate() {
  bool ret = true;
  ret &= initializeSubdomains(true);
  initializeWindowManager();
  callInitializeCallbacks();
  ret &= initializeSubdomains(false);
  mInitialized = true;
  return ret;
}

bool OpenGLGraphicsDomain::tick() {
  tickFPS();
  mTimeDrift = dt_sec();
  return true;
}

bool OpenGLGraphicsDomain::startPrivate() {
  if (!mRunning) {
    mRunning = true;
    bool ret = true;
    startFPS();
    callStartCallbacks();
    return ret;
  } else {
    return true;
  }
}

bool OpenGLGraphicsDomain::stopPrivate() {
  bool ret = true;
  callStopCallbacks();

  ret &= cleanupSubdomains(true);

  onExit(); // user defined
  postOnExit();

  ret &= cleanupSubdomains(false);
  mRunning = false;
  return ret;
}

bool OpenGLGraphicsDomain::cleanupPrivate() {
  callCleanupCallbacks();
  terminateWindowManager();
  mInitialized = false;
  return true;
}

/// Window Domain ----------------------

GLFWOpenGLWindowDomain::GLFWOpenGLWindowDomain() {
  mWindow = std::make_unique<Window>();
}

bool GLFWOpenGLWindowDomain::init(ComputationDomain *parent) {
  //  if (strcmp(typeid(*parent).name(), typeid(OpenGLGraphicsDomain).name()) ==
  //  0) {
  //    mGraphics = &static_cast<OpenGLGraphicsDomain *>(parent)->graphics();
  //  }
  bool ret = true;
  assert(dynamic_cast<OpenGLGraphicsDomain *>(parent));
  mGraphics = std::make_unique<Graphics>();

  ret &= initializeSubdomains(true);
  mParent = static_cast<OpenGLGraphicsDomain *>(parent);
  //  if (!mWindow) {
  //    mWindow = std::make_unique<Window>();
  //  }
  if (!mGraphics) {
    mGraphics = std::make_unique<Graphics>();
  }
  mWindow->mWindowProperties =
      dynamic_cast<OpenGLGraphicsDomain *>(parent)->nextWindowProperties;
  if (!mWindow->created()) {
    ret &= mWindow->create();
    mGraphics->init();
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

  mWindow->destroy();
  //  if (mWindow) {
  //    mWindow = nullptr;
  //  }
  if (mGraphics) {
    mGraphics = nullptr;
  }
  mInitialized = false;
  return ret;
}
