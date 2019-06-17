#ifndef COMPUTATIONDOMAIN_H
#define COMPUTATIONDOMAIN_H

#include <stack>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>
#include <mutex>
#include <cassert>


namespace al
{
class SynchronousDomain;

class ComputationDomain
{
public:
  virtual bool initialize(ComputationDomain *parent = nullptr) = 0;

  virtual bool cleanup(ComputationDomain *parent = nullptr) = 0;

  bool initializeSubdomains(bool pre = false);
  bool tickSubdomains(bool pre = false);
  bool cleanupSubdomains(bool pre = false);

  /**
   * @brief callInitializeCallbacks should be called by children of this class after the domain has been initialized
   */
  void callInitializeCallbacks() {
    for (auto callback: mInitializeCallbacks) {
      callback(this);
    }
  }

  /**
   * @brief callInitializeCallbacks should be called by children of this class before the domain has been cleaned up
   */
  void callCleanupCallbacks() {
    for (auto callback: mCleanupCallbacks) {
      callback(this);
    }
  }

  void registerInitializeCallback(std::function<void(ComputationDomain *)> callback) {
    mInitializeCallbacks.push_back(callback);
  }

  void registerCleanupCallback(std::function<void(ComputationDomain *)> callback) {
    mCleanupCallbacks.push_back(callback);
  }

  template<class DomainType>
  std::shared_ptr<DomainType> newSubDomain(bool prepend = false);

  void removeSubDomain(std::shared_ptr<SynchronousDomain> subDomain);

  void lock() { mSubdomainLock.lock();}
  void unlock() { mSubdomainLock.unlock(); }

protected:
  std::mutex mSubdomainLock; // It is the domain's responsibility to lock and unlock while processing.

private:
  std::vector<std::pair<std::shared_ptr<SynchronousDomain>, bool>> mSubDomainList;
  std::vector<std::function<void(ComputationDomain *)>> mInitializeCallbacks;
  std::vector<std::function<void(ComputationDomain *)>> mCleanupCallbacks;

};


class SynchronousDomain : public ComputationDomain
{
  friend class ComputationDomain;
public:

  virtual bool tick() { return true;}
};

class AsynchronousDomain : public ComputationDomain
{
public:

  virtual bool start() = 0;

  virtual bool stop() = 0;

  /**
   * @brief callInitializeCallbacks should be called by children of this class after the domain has been set up to start, before going into the blocking loop
   */
  void callStartCallbacks() {
    for (auto callback: mStartCallbacks) {
      callback(this);
    }
  }

  /**
   * @brief callInitializeCallbacks should be called by children of this class on the stop request, before the domain has been stopped
   */
  void callStopCallbacks() {
    for (auto callback: mStopCallbacks) {
      callback(this);
    }
  }

private:
  std::vector<std::function<void(ComputationDomain *)>> mStartCallbacks;
  std::vector<std::function<void(ComputationDomain *)>> mStopCallbacks;
};

template<class DomainType>
std::shared_ptr<DomainType> ComputationDomain::newSubDomain(bool prepend) {
//  std::lock_guard<std::mutex> lk(mSubdomainLock);
  // Only Synchronous domains are allowed as subdomains
  auto newDomain = std::make_shared<DomainType>();
  assert(dynamic_cast<SynchronousDomain *>(newDomain.get()));
  if (newDomain) {
    if (newDomain->initialize(this)) {
      mSubDomainList.push_back({newDomain, prepend});
    } else {
      newDomain = nullptr;
    }
  }
  return newDomain;
}

}


#endif // COMPUTATIONDOMAIN_H
