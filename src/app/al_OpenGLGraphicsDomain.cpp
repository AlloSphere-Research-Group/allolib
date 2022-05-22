#include "al/app/al_OpenGLGraphicsDomain.hpp"

#include <cstring>
#include <iostream>

#include "al/io/al_Window.hpp"

using namespace al;

OpenGLGraphicsDomain::OpenGLGraphicsDomain() {
  ComputationDomain::addPublicDomain(this, "graphics");
}

bool OpenGLGraphicsDomain::init(ComputationDomain *parent) {
  if (!mInitialized) {
    bool ret = true;
    ret &= initializeSubdomains(true);
    initializeWindowManager();
    callInitializeCallbacks();
    ret &= initializeSubdomains(false);
    mInitialized = true;
    return ret;
  }
  std::cout << "OpenGLGraphicsDomain already initialized" << std::endl;
  return true;
}

bool OpenGLGraphicsDomain::start() {
  mRunning = true;
  bool ret = true;
  startFPS();
  callStartCallbacks();
  if (!ret) {
    mRunning = false;
  }
  onCreate();

  bool subDomainsOk = true;
  while (mRunning && !shouldStop() && subDomainsOk) {
    //      std::cout << "tick graphics" << std::endl;
    processDomainAddRemoveQueues(this);
    subDomainsOk &= tickSubdomains(true);
    tickFPS();
    mTimeDrift = dt_sec();
    subDomainsOk &= tickSubdomains(false);
  }
  stop();

  //  cleanup();

  return ret;
}

bool OpenGLGraphicsDomain::stop() {
  bool ret = true;
  if (mRunning) {
    mRunning = false;
  } else {
    callStopCallbacks();

    onExit(); // user defined
    postOnExit();
  }
  return ret;
}

bool OpenGLGraphicsDomain::cleanup(ComputationDomain *parent) {
  bool ret = true;
  if (mInitialized) {
    for (GPUObject *obj : mObjects) {
      obj->destroy();
    }
    ret &= cleanupSubdomains(true);
    ret &= cleanupSubdomains(false);
    callCleanupCallbacks();
    terminateWindowManager();
    mInitialized = false;
  }

  return ret;
}

bool OpenGLGraphicsDomain::addSubDomain(
    std::shared_ptr<SynchronousDomain> subDomain, bool prepend) {

  if (!mInitialized || !mRunning) {
    AsynchronousDomain::addSubDomain(subDomain, prepend);
    return true;
  } else {
    return addSubDomainAsync(subDomain, prepend);
  }
  return true;
}

bool OpenGLGraphicsDomain::removeSubDomain(
    std::shared_ptr<SynchronousDomain> subDomain) {
  if (!mRunning) {
    AsynchronousDomain::removeSubDomain(subDomain);
  } else {
    return removeSubDomainAsync(subDomain);
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
      if (domainToInsert.subDomain) {
        if (!domainToInsert.subDomain->init(domain)) {
          domainToInsert.subDomainInsertStatus->set_value(false);
          std::cerr << "ERROR initializing Window domain. Not injecting."
                    << std::endl;
        } else {
          domain->mSubDomainList.push_back(
              {domainToInsert.subDomain, domainToInsert.prepend});
          domainToInsert.subDomainInsertStatus->set_value(true);
        }
      }
    }
    domain->mSubdomainsToInsert.clear();
  }
  {
    std::unique_lock<std::mutex> lk(domain->mSubDomainRemoveLock);
    for (const auto &domainToRemove : domain->mSubdomainsToRemove) {
      auto it = domain->mSubDomainList.begin();
      while (it->first != domainToRemove.subDomain &&
             it != domain->mSubDomainList.end()) {
        it++;
      }
      if (it != domain->mSubDomainList.end()) {
        //          domainToRemove.subDomain->cleanup();
        domain->mSubDomainList.erase(it);
        domainToRemove.subDomainRemoveStatus->set_value(true);
      } else {
        std::cerr << "Domain not found for removal." << std::endl;
        domainToRemove.subDomainRemoveStatus->set_value(false);
      }
      domain->mSubdomainsToRemove.clear();
    }
  }
}

bool OpenGLGraphicsDomain::addSubDomainAsync(
    std::shared_ptr<SynchronousDomain> subDomain, bool prepend,
    uint32_t timeoutMs) {
  std::shared_ptr<std::promise<bool>> status =
      std::make_shared<std::promise<bool>>();

  auto statusFuture = status->get_future();
  std::unique_lock<std::mutex> lk(mSubDomainInsertLock);
  mSubdomainsToInsert.emplace_back(
      SubDomainToInsert{subDomain, prepend, status});
  if (statusFuture.wait_for(std::chrono::milliseconds(timeoutMs)) ==
      std::future_status::ready) {
    return statusFuture.get();
  } else {
    return false;
  }
  return true;
}

bool OpenGLGraphicsDomain::removeSubDomainAsync(
    std::shared_ptr<SynchronousDomain> subDomain, uint32_t timeoutMs) {
  std::shared_ptr<std::promise<bool>> status =
      std::make_shared<std::promise<bool>>();
  auto statusFuture = status->get_future();
  std::unique_lock<std::mutex> lk(mSubDomainInsertLock);
  mSubdomainsToRemove.emplace_back(SubDomainToRemove{subDomain, status});
  if (statusFuture.wait_for(std::chrono::milliseconds(timeoutMs)) ==
      std::future_status::ready) {
    return statusFuture.get();
  } else {
    return false;
  }
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
