#ifndef COMPUTATIONDOMAIN_H
#define COMPUTATIONDOMAIN_H

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <stack>
#include <vector>

#include "al/app/al_NodeConfiguration.hpp"
#include "al/ui/al_Parameter.hpp"

namespace al {
class SynchronousDomain;

/**
 * @brief ComputationDomain class
 * @ingroup App
 */
class ComputationDomain {
public:
  virtual ~ComputationDomain() {}
  /**
   * @brief initialize
   * @param parent
   * @return true if init succeeded
   *
   * Multiple calls to init() should be allowed.
   * You should always call this function within child classes to esnure
   * internal state is correct
   */
  virtual bool init(ComputationDomain *parent = nullptr);

  /**
   * @brief cleanup
   * @param parent
   * @return true if cleanup succesfull
   *
   * You should always call this function within child classes to esnure
   * internal state is correct
   */
  virtual bool cleanup(ComputationDomain *parent = nullptr);

  template <class DomainType>
  /**
   * @brief Add a synchronous domain to this domain
   * @param prepend Determines whether the sub-domain should run before or after
   * this domain
   * @return the created domain.
   *
   * The domain specified by DomainType must inherit from SynchronousDomian.
   * It will trigger an std::runtime_error exception if it doesn't.
   * This operation is thread safe if the domain can pause
   * itself, but if it is blocking, so there are no guarantees that it will not
   * interefere with the timely running of the domain.
   *
   * The domain will be initialized if this domain has been initialized.
   *
   * Will return nullptr if this domain is running and the sub domain failed
   * initialization.
   *
   * This function calls addSubDomain() once the domain has been created.
   */
  std::shared_ptr<DomainType> newSubDomain(bool prepend = false);

  /**
   * @brief Inserts subDomain as a subdomain of this class
   * @param subDomain sub domain to insert
   * @param prepend determines if subdomainshould run before or after this
   * domain
   *
   * It is the caller's responsibility to ensure the domain is initialized and
   * ready if this domain is already running.
   *
   * This function is thread safe, and will block until the domain has been
   * added. The addtion takes place when the parent unlocks the subdomains, at
   * the end of calls to tickSubdomains().
   * It may block indefinitely if the parent domain does not release the
   * sub domain locks.
   */
  void addSubDomain(std::shared_ptr<SynchronousDomain> subDomain,
                    bool prepend = false);

  /**
   * @brief Remove a subdomain
   * @param subDomain
   *
   * This operation is thread safe, but it might block causing drops or missed
   * deadlines for the parent domain. If this is a problem, the domain should be
   * stopped prior to adding/removeing sub-domains.
   */
  void removeSubDomain(std::shared_ptr<SynchronousDomain> subDomain);

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

  void setTimeDelta(double delta) { mTimeDrift = delta; }

  Capability getCapabilities() { return mCapabilities; }

  /**
   * @brief register callbacks to be called in the init() function
   * @param callback
   */
  void
  registerInitializeCallback(std::function<void(ComputationDomain *)> callback);

  /**
   * @brief register callbacks to be called in the cleanup() function
   * @param callback
   */
  void
  registerCleanupCallback(std::function<void(ComputationDomain *)> callback);

  /**
   * @brief Return a list of parameters that control this domain
   * @return list of parameters
   *
   * The parameters provided here provide runtime "continuous" parameters,
   * for example like audio gain or eye separation. There should be a clear
   * distinction between values that need to be set on domain intialization
   * that must remain immutable during domain operation and parameters
   * provided here that provide continuous adjustment to the domain's
   * operation.
   */
  std::vector<ParameterMeta *> parameters() { return mParameters; }

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
  void callInitializeCallbacks();

  /**
   * @brief callInitializeCallbacks should be called by children of this class
   * before the domain has been cleaned up
   */
  void callCleanupCallbacks();

  double mTimeDrift{0.0};
  std::vector<std::pair<std::shared_ptr<SynchronousDomain>, bool>>
      mSubDomainList;

  // Add parameters for domain control here
  std::vector<ParameterMeta *> mParameters;

protected:
  bool mInitialized{false};

private:
  std::vector<std::function<void(ComputationDomain *)>> mInitializeCallbacks;
  std::vector<std::function<void(ComputationDomain *)>> mCleanupCallbacks;

  std::mutex mSubdomainLock;

  Capability mCapabilities;
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
   * Assumes that init() has already been called.
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
  void callStartCallbacks();

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
  auto newDomain = std::make_shared<DomainType>();
  if (!dynamic_cast<SynchronousDomain *>(newDomain.get())) {
    // Only Synchronous domains are allowed as subdomains
    throw std::runtime_error(
        "Subdomain must be a subclass of SynchronousDomain");
  }
  if (mInitialized) {
    if (newDomain->init(this)) {
      addSubDomain(newDomain, prepend);
    } else {
      newDomain = nullptr;
    }
  } else {
    addSubDomain(newDomain, prepend);
  }
  return newDomain;
}

} // namespace al

#endif // COMPUTATIONDOMAIN_H
