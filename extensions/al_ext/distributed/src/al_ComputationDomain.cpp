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

void al::BaseCompositeApp::start() {
  for (auto domain: mDomainList) {
    if (!domain->initialize()) {
      std::cerr << "ERROR initializing domain " << std::endl;
    } else {
      if (strcmp(typeid(*domain).name(), typeid(GraphicsDomain).name()) == 0) {
        dynamic_cast<GraphicsDomain *>(domain.get())->onInit = std::bind(&BaseCompositeApp::onInit, this);
        dynamic_cast<GraphicsDomain *>(domain.get())->onCreate = std::bind(&BaseCompositeApp::onCreate, this);
        dynamic_cast<GraphicsDomain *>(domain.get())->onDraw = std::bind(&BaseCompositeApp::onDraw, this, std::placeholders::_1);
      } else if (strcmp(typeid(*domain).name(), typeid(AudioDomain).name()) == 0) {
        dynamic_cast<AudioDomain *>(domain.get())->onSound = std::bind(&BaseCompositeApp::onSound, this, std::placeholders::_1);
      } else if (strcmp(typeid(*domain).name(), typeid(OSCDomain).name()) == 0) {
        dynamic_cast<OSCDomain *>(domain.get())->onMessage = std::bind(&BaseCompositeApp::onMessage, this, std::placeholders::_1);
      }

    }
  }

  for (auto &domain: mDomainList) {
    mRunningDomains.push(domain);
    if (!domain->start()) {
      std::cerr << "ERROR starting domain " << std::endl;
    }
  }

  while (mRunningDomains.size() > 0 ) {

    if (!mRunningDomains.top()->stop()) {
      std::cerr << "ERROR stopping domain " << std::endl;
    }
    mRunningDomains.pop();
  }

  for (auto &domain: mDomainList) {
    if (!domain->cleanup()) {
      std::cerr << "ERROR cleaning up domain " << std::endl;
    }
  }
}
