#ifndef COMPUTATIONDOMAIN_H
#define COMPUTATIONDOMAIN_H

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <stack>
#include <vector>

namespace al {
class SynchronousDomain;

/**
 * @brief ComputationDomain class
 * @ingroup App
 */
class ComputationDomain {
 public:
  /**
   * @brief initialize
   * @param parent
   * @return
   *
   * Multiple calls to initialize() should be allowed.
   */
  virtual bool initialize(ComputationDomain *parent = nullptr);

  virtual bool cleanup(ComputationDomain *parent = nullptr);

  template <class DomainType>
  /**
   * @brief Add a synchronous domain to this domain
   * @param prepend Determines whether the sub-domain should run before or after
   * this domain
   * @return the created domain.
   *
   * The domain specified by DomainType must inherit from SynchronousDomian.
   * It will trigger an assertion if not (will have undefined behavior for
   * Release builds) This operation is thread safe if the domain can pause
   * itself, but it is blocking, so there are no guarantees that it will not
   * interefere with the timely running of the domain.
   */
  std::shared_ptr<DomainType>
  newSubDomain(bool prepend = false);

  /**
   * @brief Remove a subdomain
   * @param subDomain
   *
   * This operation is thread safe, but it might block causing drops or missed
   * deadlines for the parent domain. If this is a problem, the domain should be
   * stopped prior to adding/removeing sub-domains.
   */
  void removeSubDomain(std::shared_ptr<SynchronousDomain> subDomain);

  //  void lock() { mSubdomainLock.lock();}
  //  void unlock() { mSubdomainLock.unlock(); }

  /**
   * @brief Return time delta with respect to previous processing pass for this
   * domain
   * @return time delta
   *
   * The time delta may be set internally by the domain so that it is available
   * whenever the domain ticks. It might be 0 for domains that don't support
   * this functionality or that are tied to strict hardware clocks where this
   * information is not necessary
   */
  double timeDelta() { return mTimeDrift; }

  void registerInitializeCallback(
      std::function<void(ComputationDomain *)> callback) {
    mInitializeCallbacks.push_back(callback);
  }

  void registerCleanupCallback(
      std::function<void(ComputationDomain *)> callback) {
    mCleanupCallbacks.push_back(callback);
  }

 protected:
  /**
   * @brief initializeSubdomains should be called within the domain's
   * initialization function
   * @param pre initialize prepended domains if true, otherwise appended
   * domains.
   * @return true if all initializations sucessful.
   *
   * You must call this function twice: once for prepended and then for appended
   * domains.
   */
  bool initializeSubdomains(bool pre = false);

  /**
   * @brief execute subdomains
   * @param pre execute prepended domains if true, otherwise appended domains.
   * @return true if all execution sucessful.
   *
   * You must call this function twice: once for prepended and then for appended
   * domains.
   */
  bool tickSubdomains(bool pre = false);

  /**
   * @brief cleanup subdomains
   * @param pre cleanup prepended domains if true, otherwise appended domains.
   * @return true if all cleanup sucessful.
   *
   * You must call this function twice: once for prepended and then for appended
   * domains.
   */
  bool cleanupSubdomains(bool pre = false);

  /**
   * @brief callInitializeCallbacks should be called by children of this class
   * after the domain has been initialized
   */
  void callInitializeCallbacks() {
    for (auto callback : mInitializeCallbacks) {
      callback(this);
    }
  }

  /**
   * @brief callInitializeCallbacks should be called by children of this class
   * before the domain has been cleaned up
   */
  void callCleanupCallbacks() {
    for (auto callback : mCleanupCallbacks) {
      callback(this);
    }
  }

 protected:
  std::mutex mSubdomainLock;  // It is the domain's responsibility to lock and
                              // unlock while processing.
  double mTimeDrift{0.0};
  std::vector<std::pair<std::shared_ptr<SynchronousDomain>, bool>>
      mSubDomainList;

 private:
  std::vector<std::function<void(ComputationDomain *)>> mInitializeCallbacks;
  std::vector<std::function<void(ComputationDomain *)>> mCleanupCallbacks;
};

class SynchronousDomain : public ComputationDomain {
  friend class ComputationDomain;

 public:
  /**
   * @brief Execute a pass of the domain.
   * @return true if execution of the domain succeeded
   */
  virtual bool tick();
};

class AsynchronousDomain : public ComputationDomain {
 public:
  /**
   * @brief start the asyncrhonous execution of the domain
   * @return true if start was successful
   *
   * Assumes that initialize() has already been called.
   */
  virtual bool start() = 0;

  /**
   * @brief stop the asyncrhonous execution of the domain
   * @return true if stop was successful
   *
   * Domains should be written so that either start() or cleanup()
   * work after calling stop()
   */
  virtual bool stop() = 0;

 protected:
  /**
   * @brief callStartCallbacks should be called by children of this class after
   * the domain has been set up to start, before going into the blocking loop
   */
  void callStartCallbacks() {
    for (auto callback : mStartCallbacks) {
      callback(this);
    }
  }

  /**
   * @brief callStopCallbacks should be called by children of this class on the
   * stop request, before the domain has been stopped
   */
  void callStopCallbacks() {
    for (auto callback : mStopCallbacks) {
      callback(this);
    }
  }

 private:
  std::vector<std::function<void(ComputationDomain *)>> mStartCallbacks;
  std::vector<std::function<void(ComputationDomain *)>> mStopCallbacks;
};

template <class DomainType>
std::shared_ptr<DomainType> ComputationDomain::newSubDomain(bool prepend) {
  //  std::lock_guard<std::mutex> lk(mSubdomainLock);
  // Only Synchronous domains are allowed as subdomains
  auto newDomain = std::make_shared<DomainType>();
  assert(dynamic_cast<SynchronousDomain *>(newDomain.get()));
  if (newDomain) {
    mSubDomainList.push_back({newDomain, prepend});
//    if (newDomain->initialize(this)) {
//    } else {
//      newDomain = nullptr;
//    }
  }
  return newDomain;
}

}  // namespace al

#endif  // COMPUTATIONDOMAIN_H
