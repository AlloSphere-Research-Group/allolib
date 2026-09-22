#ifndef INCLUDE_AL_RUNTIME_HPP
#define INCLUDE_AL_RUNTIME_HPP

#include <atomic>
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
 * @brief Owns root Modules and their start / pump / stop / cleanup cycle.
 * @ingroup App
 *
 * Runtime owns every loop. When no root blocks inside start(), Runtime pumps
 * Main-schedule modules (poll + tickFrame) or idles until quit() for
 * Callback-only graphs. Legacy App path: any blocksInStart() root keeps the
 * sequential blocking start stack.
 *
 * App and DistributedApp use Runtime for lifecycle. Callbacks (onCreate,
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
  std::atomic<bool> mQuit{false};
};

} // namespace al

#endif // INCLUDE_AL_RUNTIME_HPP
