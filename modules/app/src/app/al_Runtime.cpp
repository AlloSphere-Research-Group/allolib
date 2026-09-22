#include "al/app/al_Runtime.hpp"

#include <algorithm>
#include <chrono>
#include <thread>

using namespace al;

void Runtime::removeDomain(std::shared_ptr<AsynchronousDomain> domain) {
  auto it = std::find(mDomainList.begin(), mDomainList.end(), domain);
  if (it != mDomainList.end()) {
    mDomainList.erase(it);
  }
}

void Runtime::initialize(std::function<void(AsynchronousDomain *)> connect) {
  for (auto &domain : mDomainList) {
    if (connect) {
      connect(domain.get());
    }
    if (!domain->init()) {
      std::cerr << "ERROR initializing domain " << std::endl;
    }
  }
}

void Runtime::run(std::function<void()> beforeStart) {
  mQuit = false;

  if (beforeStart) {
    beforeStart();
  }

  bool legacyBlocking = false;
  for (auto &domain : mDomainList) {
    if (domain->blocksInStart()) {
      legacyBlocking = true;
      break;
    }
  }

  if (legacyBlocking) {
    // App / DistributedApp path: a root may block inside start() (graphics
    // Self loop, console, or KeepAlive).
    for (auto &domain : mDomainList) {
      mRunningDomains.push(domain);
      if (!domain->start()) {
        std::cerr << "ERROR starting domain " << std::endl;
        break;
      }
    }

    while (!mRunningDomains.empty()) {
      if (!mRunningDomains.top()->stop()) {
        std::cerr << "ERROR stopping domain " << std::endl;
      }
      mRunningDomains.pop();
    }
    return;
  }

  // Cooperative path: no root blocks in start(); Runtime owns the pump.
  for (auto &domain : mDomainList) {
    mRunningDomains.push(domain);
    if (!domain->start()) {
      std::cerr << "ERROR starting domain " << std::endl;
      break;
    }
  }

  std::vector<AsynchronousDomain *> mainPump;
  mainPump.reserve(mDomainList.size());
  for (auto &domain : mDomainList) {
    if (domain->schedule() == ModuleSchedule::Main) {
      mainPump.push_back(domain.get());
    }
  }

  if (!mainPump.empty()) {
    bool ok = true;
    while (ok && !shouldQuit()) {
      for (auto *mod : mainPump) {
        mod->poll();
        if (!mod->tickFrame()) {
          ok = false;
          quit();
          break;
        }
      }
    }
  } else {
    // Callback-only graph (e.g. audio): idle until quit().
    while (!shouldQuit()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  while (!mRunningDomains.empty()) {
    if (!mRunningDomains.top()->stop()) {
      std::cerr << "ERROR stopping domain " << std::endl;
    }
    mRunningDomains.pop();
  }
}

void Runtime::quit() {
  mQuit = true;
  for (auto &domain : mDomainList) {
    domain->quit();
  }
}

bool Runtime::shouldQuit() const {
  if (mQuit.load()) {
    return true;
  }
  for (auto &domain : mDomainList) {
    if (domain->shouldQuit()) {
      return true;
    }
  }
  return false;
}

void Runtime::cleanup() {
  for (auto &domain : mDomainList) {
    if (!domain->cleanup()) {
      std::cerr << "ERROR cleaning up domain " << std::endl;
    }
  }
}
