#ifndef INCLUDE_AL_RUNTIME_HPP
#define INCLUDE_AL_RUNTIME_HPP

#include <functional>
#include <iostream>
#include <memory>
#include <stack>
#include <vector>

#include "al/app/al_ComputationDomain.hpp"

/** @defgroup App Application building tools
 *
 */

namespace al {

/**
 * @brief Owns the asynchronous domain graph and its start/stop/cleanup cycle.
 * @ingroup App
 *
 * App and DistributedApp use Runtime for lifecycle. Domain callbacks (onCreate,
 * onSound, …) are still wired by the app recipe via initialize().
 */
class Runtime {
public:
  template <class DomainType> std::shared_ptr<DomainType> newDomain() {
    auto domain = std::make_shared<DomainType>();
    mDomainList.push_back(domain);
    return domain;
  }

  /// Remove a previously added asynchronous domain. No-op if not present.
  void removeDomain(std::shared_ptr<AsynchronousDomain> domain);

  /// Init each domain, then run the start/stop/cleanup cycle.
  /// \p beforeStart is invoked after init, before any domain start().
  void run(std::function<void()> beforeStart = {});

  /// Request quit. Graphics domain is notified when present so its loop exits.
  void quit();
  bool shouldQuit() const;
  bool isRunning() const { return !mRunningDomains.empty(); }

  /// Init domains and invoke \p connect(domain) for each (typically callback wiring).
  void initialize(std::function<void(AsynchronousDomain *)> connect = {});

  void cleanup();

  std::vector<std::shared_ptr<AsynchronousDomain>> &domains() {
    return mDomainList;
  }
  std::vector<std::shared_ptr<AsynchronousDomain>> const &domains() const {
    return mDomainList;
  }

private:
  std::vector<std::shared_ptr<AsynchronousDomain>> mDomainList;
  std::stack<std::shared_ptr<AsynchronousDomain>> mRunningDomains;
};

} // namespace al

#endif // INCLUDE_AL_RUNTIME_HPP
