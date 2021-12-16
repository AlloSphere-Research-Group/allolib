#include <cstring>
#include <iostream>

#include "al/app/al_ComputationDomain.hpp"

using namespace al;

bool ComputationDomain::initializeSubdomains(bool pre) {
  bool ret = true;
  for (auto subDomain : mSubDomainList) {
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
  for (auto subDomain : mSubDomainList) {
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
  for (auto subDomain : mSubDomainList) {
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

bool ComputationDomain::init(ComputationDomain *parent) {
  bool ret = initializeSubdomains(true);
  ret &= initializeSubdomains(false);
  mInitialized = true;
  return ret;
}

bool ComputationDomain::cleanup(ComputationDomain *parent) {
  mInitialized = false;
  bool ret = cleanupSubdomains(true);
  ret &= cleanupSubdomains(false);
  return ret;
}

void ComputationDomain::addSubDomain(
    std::shared_ptr<SynchronousDomain> subDomain, bool prepend) {
  std::lock_guard<std::mutex> lk(mSubdomainLock);
  mSubDomainList.push_back({subDomain, prepend});
}

void ComputationDomain::removeSubDomain(
    std::shared_ptr<SynchronousDomain> subDomain) {
  // Only Synchronous domains are allowed as subdomains
  for (auto existingSubDomain = mSubDomainList.begin();
       existingSubDomain != mSubDomainList.end(); existingSubDomain++) {
    if (existingSubDomain->first == subDomain) {
      existingSubDomain->first->cleanup();
      mSubDomainList.erase(existingSubDomain);
      break;
    }
  }
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
