#include <iostream>
#include <cstring>

#include "al_ext/distributed/al_ComputationDomain.hpp"

using namespace al;

bool ComputationDomain::initializeSubdomains(bool pre)
{
  bool ret = true;
  for (auto subDomain: mSubDomainList) {
    if (subDomain.second == pre) {
      auto syncSubDomain = std::dynamic_pointer_cast<SynchronousDomain>(subDomain.first);
      if (syncSubDomain) {
        ret &= syncSubDomain->initialize(this);
      }
    }
  }
  return ret;
}

bool ComputationDomain::tickSubdomains(bool pre) {
  bool ret = true;
  for (auto subDomain: mSubDomainList) {
    if (subDomain.second == pre) {
      auto syncSubDomain = std::dynamic_pointer_cast<SynchronousDomain>(subDomain.first);
      if (syncSubDomain) {
        ret &= syncSubDomain->tick();
      }
    }
  }
  return ret;
}

bool ComputationDomain::cleanupSubdomains(bool pre)
{
  bool ret = true;
  for (auto subDomain: mSubDomainList) {
    if (subDomain.second == pre) {
      auto syncSubDomain = std::dynamic_pointer_cast<SynchronousDomain>(subDomain.first);
      if (syncSubDomain) {
        ret &= syncSubDomain->cleanup(this);
      }
    }
  }
  return ret;
}

void ComputationDomain::removeSubDomain(std::shared_ptr<SynchronousDomain> subDomain)
{
  // Only Synchronous domains are allowed as subdomains
  for (auto existingSubDomain = mSubDomainList.begin(); existingSubDomain != mSubDomainList.end(); existingSubDomain++) {
    if (existingSubDomain->first == subDomain) {
      existingSubDomain->first->cleanup();
      mSubDomainList.erase(existingSubDomain);
      break;
    }
  }
}
