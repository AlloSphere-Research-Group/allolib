#include <cstring>
#include <iostream>

#include "al/app/al_ComputationDomain.hpp"

using namespace al;

std::vector<std::pair<ComputationDomain *, std::string>>
    ComputationDomain::mPublicDomains;
std::mutex ComputationDomain::mPublicDomainsLock;

void DomainMember::registerWithDomain(ComputationDomain *domain) {
  if (!domain) {
    domain = getDefaultDomain();
  }
  if (!domain) {
    std::cerr << "ERROR could not register object with domain" << std::endl;
    return;
  }
  domain->registerObject(this);
  mParentDomain = domain;
}

void DomainMember::unregisterFromDomain(ComputationDomain *domain) {
  if (!domain) {
    if (!mParentDomain) {
      domain = getDefaultDomain();
    } else {
      domain = mParentDomain;
    }
  }
  if (!domain) {
    std::cerr << "ERROR could not find domain domain" << std::endl;
    return;
  }
  domain->unregisterObject(this);
  mParentDomain = nullptr;
}

// --------------------------------------------------

bool ComputationDomain::initializeSubdomains(bool pre) {
  bool ret = true;
  for (const auto &subDomain : mSubDomainList) {
    if (subDomain.second == pre) {
      auto syncSubDomain =
          std::dynamic_pointer_cast<SynchronousDomain>(subDomain.first);
      if (syncSubDomain) {
        ret &= syncSubDomain->init(this);
      }
    }
  }
  return ret;
}

bool ComputationDomain::tickSubdomains(bool pre) {
  bool ret = true;
  std::unique_lock<std::mutex> lk(mSubdomainLock);
  for (const auto &subDomain : mSubDomainList) {
    if (subDomain.second == pre) {
      auto syncSubDomain =
          std::dynamic_pointer_cast<SynchronousDomain>(subDomain.first);
      if (syncSubDomain) {
        syncSubDomain->mTimeDrift = mTimeDrift;
        ret &= syncSubDomain->tick();
      }
    }
  }
  return ret;
}

bool ComputationDomain::cleanupSubdomains(bool pre) {
  bool ret = true;
  for (const auto &subDomain : mSubDomainList) {
    if (subDomain.second == pre) {
      auto syncSubDomain =
          std::dynamic_pointer_cast<SynchronousDomain>(subDomain.first);
      if (syncSubDomain) {
        ret &= syncSubDomain->cleanup(this);
      }
    }
  }
  return ret;
}

void ComputationDomain::callInitializeCallbacks() {
  for (auto callback : mInitializeCallbacks) {
    callback(this);
  }
}

void ComputationDomain::callCleanupCallbacks() {
  for (auto callback : mCleanupCallbacks) {
    callback(this);
  }
}

void ComputationDomain::addPublicDomain(ComputationDomain *domain,
                                        std::string tag) {
  std::lock_guard<std::mutex> lk(mPublicDomainsLock);
  if (std::find(mPublicDomains.begin(), mPublicDomains.end(),
                std::pair<ComputationDomain *, std::string>{domain, tag}) ==
      mPublicDomains.end()) {
    mPublicDomains.push_back({domain, tag});
  } else {
    std::cout << "Warning Domain already registered." << std::endl;
  }
}

bool ComputationDomain::init(ComputationDomain *parent) {
  if (!mInitialized) {
    bool ret = initializeSubdomains(true);
    callInitializeCallbacks();
    ret &= initializeSubdomains(false);
    mInitialized = true;
    return ret;
  }
  return true;
}

bool ComputationDomain::cleanup(ComputationDomain *parent) {
  mInitialized = false;
  bool ret = cleanupSubdomains(true);
  ret &= cleanupSubdomains(false);
  return ret;
}

bool ComputationDomain::addSubDomain(
    std::shared_ptr<SynchronousDomain> subDomain, bool prepend) {
  std::lock_guard<std::mutex> lk(mSubdomainLock);
  mSubDomainList.push_back({subDomain, prepend});
  return true;
}

bool ComputationDomain::removeSubDomain(
    std::shared_ptr<SynchronousDomain> subDomain) {
  // Only Synchronous domains are allowed as subdomains
  if (!subDomain) {
    for (const auto &existingSubDomain : mSubDomainList) {
      existingSubDomain.first->cleanup();
    }
    mSubDomainList.clear();
    return true;
  }
  for (auto existingSubDomain = mSubDomainList.begin();
       existingSubDomain != mSubDomainList.end(); existingSubDomain++) {
    if (existingSubDomain->first == subDomain) {
      existingSubDomain->first->cleanup();
      mSubDomainList.erase(existingSubDomain);
      return true;
    }
  }
  return false;
}

ComputationDomain *ComputationDomain::getDomain(std::string tag, size_t index) {
  std::lock_guard<std::mutex> lk(mPublicDomainsLock);
  for (const auto &domain : mPublicDomains) {
    if (domain.second == tag) {
      if (index == 0) {
        return domain.first;
      } else {
        index--;
      }
    }
  }
  return nullptr;
}

void ComputationDomain::registerInitializeCallback(
    std::function<void(ComputationDomain *)> callback) {
  mInitializeCallbacks.push_back(callback);
}

void ComputationDomain::registerCleanupCallback(
    std::function<void(ComputationDomain *)> callback) {
  mCleanupCallbacks.push_back(callback);
}

bool SynchronousDomain::tick() {
  bool ret = tickSubdomains(true);
  ret &= tickSubdomains(false);
  return ret;
}

void AsynchronousDomain::callStartCallbacks() {
  for (auto callback : mStartCallbacks) {
    callback(this);
  }
}

// ------------------------------------------------------

// bool AsynchronousThreadDomain::start() {
//  if (mAsyncThread) {
//    return true;
//  }
//  mAsyncThread = std::make_unique<std::thread>([this]() { this->start(); });
//  return true;
//}

std::future<bool> &AsynchronousThreadDomain::waitForDomain() {
  // should std::move be used here to avoid user error?
  return mDomainAsyncResult;
}

// bool AsynchronousThreadDomain::stop() {
//  bool ret = true;]
//  if (mAsyncThread) {
//    mAsyncThread->join();
//  }
//  return ret;
//}

// bool AsynchronousThreadDomain::runningAsync() {
//  return mAsyncThread != nullptr;
//}
