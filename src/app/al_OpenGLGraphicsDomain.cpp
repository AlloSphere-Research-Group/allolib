#include "al/app/al_OpenGLGraphicsDomain.hpp"

#include <cstring>
#include <iostream>

#include "al/io/al_Window.hpp"

using namespace al;

OpenGLGraphicsDomain::OpenGLGraphicsDomain() {
  ComputationDomain::addPublicDomain(this, "graphics");
}

bool OpenGLGraphicsDomain::init(ComputationDomain *parent) {
  if (!mDomainThread) {

    mDomainAsyncInitPromise = std::promise<bool>();
    mDomainThread = std::make_unique<std::thread>(
        OpenGLGraphicsDomain::domainThreadFunction, this);
    // Wait for init to be done on thread;
    mDomainAsyncInit = mDomainAsyncInitPromise.get_future();
    return mDomainAsyncInit.get();
  }
  std::cout << "OpenGLGraphicsDomain already initialized" << std::endl;
  return true;
}

bool OpenGLGraphicsDomain::start() {
  if (!mRunning) {
    mDomainAsyncResultPromise = std::promise<bool>();
    {
      std::lock_guard<std::mutex> lksig(mDomainSignalLock);
      mDomainCommand = CommandType::START;
    }

    mDomainAsyncResult = mDomainAsyncResultPromise.get_future();
    return true;
  }

  std::cout << "OpenGLGraphicsDomain already started" << std::endl;
  return true;
}

bool OpenGLGraphicsDomain::stop() {
  if (mRunning) {
    mShouldStopDomain = true;

    {
      std::lock_guard<std::mutex> lksig(mDomainSignalLock);
      mDomainCommand = CommandType::STOP;
    }
    std::unique_lock<std::mutex> lk(mDomainCommandLock);
    mDomainCommandSignal.wait(lk);

    return mCommandResult;
  }
  //  std::cout << "OpenGLGraphicsDomain not running" << std::endl;
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

  if (!mInitialized) {
    std::unique_lock<std::mutex> lk(mSubDomainInsertLock);
    mSubdomainsToInsert.push_back({subDomain, prepend});
    return true;
  }
  if (!mRunning) {
    std::unique_lock<std::mutex> lk(mSubDomainInsertLock);
    mSubdomainsToInsert.push_back({subDomain, prepend});
    return true;
  } else {
    std::unique_lock<std::mutex> lk(mSubDomainInsertLock);
    mSubdomainsToInsert.push_back({subDomain, prepend});
    auto status = mSubDomainInsertSignal.wait_for(lk, std::chrono::seconds(15));
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
    mSubdomainsToRemove.push_back(subDomain);
    auto status = mSubDomainRemoveSignal.wait_for(lk, std::chrono::seconds(15));

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
  return newWindowDomain;
}

void OpenGLGraphicsDomain::closeWindow(
    std::shared_ptr<GLFWOpenGLWindowDomain> windowDomain) {
  removeSubDomain(std::static_pointer_cast<SynchronousDomain>(windowDomain));
}

bool OpenGLGraphicsDomain::registerObject(void *object) {
  // TODO validate that object is GPUObject?
  GPUObject *gpuObj = (GPUObject *)object;
  assert(gpuObj);
  mObjects.push_back(gpuObj);
  return true;
}

bool OpenGLGraphicsDomain::unregisterObject(void *object) {
  auto found = std::find(mObjects.begin(), mObjects.end(), object);
  if (found != mObjects.end()) {
    mObjects.erase(found);
  }
  return true;
}

void OpenGLGraphicsDomain::processDomainAddRemoveQueues(
    OpenGLGraphicsDomain *domain) {
  {
    std::unique_lock<std::mutex> lk(domain->mSubDomainInsertLock);
    for (const auto &domainToInsert : domain->mSubdomainsToInsert) {
      if (domainToInsert.first) {
        if (!domainToInsert.first->init(domain)) {
          std::cerr << "ERROR initializing Window domain. Not injecting."
                    << std::endl;
        } else {
          domain->mSubDomainList.push_back(domainToInsert);
        }
      }
    }
    domain->mSubDomainInsertSignal.notify_one();
    domain->mSubdomainsToInsert.clear();
  }
  {
    std::unique_lock<std::mutex> lk(domain->mSubDomainRemoveLock);
    for (const auto &domainToRemove : domain->mSubdomainsToRemove) {
      auto it = domain->mSubDomainList.begin();
      while (it->first != domainToRemove &&
             it != domain->mSubDomainList.end()) {
        it++;
      }
      if (it != domain->mSubDomainList.end()) {
        domainToRemove->cleanup();
        domain->mSubDomainList.erase(it);
      } else {
        std::cerr << "Domain not found for removal." << std::endl;
      }
      domain->mSubDomainRemoveSignal.notify_one();
      domain->mSubdomainsToRemove.clear();
    }
  }
}

void OpenGLGraphicsDomain::domainThreadFunction(OpenGLGraphicsDomain *domain) {

  domain->mDomainAsyncInitPromise.set_value(domain->initPrivate());
  processDomainAddRemoveQueues(domain);
  domain->preOnCreate();
  for (GPUObject *obj : domain->mObjects) {
    obj->create();
  }
  while (domain->mInitialized) {
    {
      std::unique_lock<std::mutex> lk(domain->mDomainSignalLock);
      if (domain->mDomainCommand != CommandType::NONE) {

        if (domain->mDomainCommand == CommandType::START) {
          if (!domain->startPrivate()) {
            domain->mDomainAsyncResultPromise.set_value(false);
          }
        } else if (domain->mDomainCommand == CommandType::STOP) {
          if (domain->mRunning) {
            domain->mDomainAsyncResultPromise.set_value(domain->stopPrivate());
          } else {
            std::cout << "Domain not running. Stop ignored" << std::endl;
          }

        } else if (domain->mDomainCommand == CommandType::CLEANUP) {
          domain->mCommandResult = domain->cleanupPrivate();
        }
        domain->mDomainCommandSignal.notify_one();
        domain->mDomainCommand = CommandType::NONE;
      }
    }

    bool subDomainsOk = true;
    while (domain->mRunning && !domain->shouldStop() && subDomainsOk) {
      //      std::cout << "tick graphics" << std::endl;
      processDomainAddRemoveQueues(domain);
      subDomainsOk &= domain->tickSubdomains(true);
      subDomainsOk &= domain->tick();
      subDomainsOk &= domain->tickSubdomains(false);
      if (!subDomainsOk) {
        domain->mShouldStopDomain = true;
        std::unique_lock<std::mutex> lk(domain->mDomainSignalLock);
        domain->mDomainCommand = CommandType::STOP;
      }
    }
  }

  for (GPUObject *obj : domain->mObjects) {
    obj->destroy();
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
    if (!ret) {
      mRunning = false;
    }
    onCreate();
    return ret;
  } else {
    return true;
  }
}

bool OpenGLGraphicsDomain::stopPrivate() {
  bool ret = true;
  callStopCallbacks();

  onExit(); // user defined
  postOnExit();

  mRunning = false;
  return ret;
}

bool OpenGLGraphicsDomain::cleanupPrivate() {
  bool ret = true;

  for (GPUObject *obj : mObjects) {
    obj->destroy();
  }
  ret &= cleanupSubdomains(true);
  ret &= cleanupSubdomains(false);
  callCleanupCallbacks();
  terminateWindowManager();
  mInitialized = false;
  return ret;
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
  if (!mGraphics) {
    mGraphics = std::make_unique<Graphics>();
  }
  ret &= initializeSubdomains(true);
  mParent = static_cast<OpenGLGraphicsDomain *>(parent);
  if (!mWindow) {
    mWindow = std::make_unique<Window>();
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
