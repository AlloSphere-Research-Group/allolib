#include "al/app/al_Runtime.hpp"

#include <algorithm>

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
  if (beforeStart) {
    beforeStart();
  }

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
}

void Runtime::quit() {
  for (auto &domain : mDomainList) {
    domain->quit();
  }
}

bool Runtime::shouldQuit() const {
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
