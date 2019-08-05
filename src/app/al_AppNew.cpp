#include "al/app/al_AppNew.hpp"

using namespace al;

void AppNew::start() {

  initializeDomains();
  onInit();

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
  onExit();
}

void AppNew::initializeDomains() {
  for (auto domain: mDomainList) {
    auto domainPtr = domain.get();
    if (strcmp(typeid(*domainPtr).name(), typeid(OpenGLGraphicsDomain).name()) == 0) {
      dynamic_cast<OpenGLGraphicsDomain *>(domainPtr)->onCreate = std::bind(&AppNew::onCreate, this);
      dynamic_cast<OpenGLGraphicsDomain *>(domainPtr)->onDraw = std::bind(&AppNew::onDraw, this, std::placeholders::_1);
      dynamic_cast<OpenGLGraphicsDomain *>(domainPtr)->app.onKeyDown  = std::bind(&AppNew::onKeyDown, this, std::placeholders::_1);
      dynamic_cast<OpenGLGraphicsDomain *>(domainPtr)->app.onKeyUp = std::bind(&AppNew::onKeyUp, this, std::placeholders::_1);
      dynamic_cast<OpenGLGraphicsDomain *>(domainPtr)->app.onMouseDown = std::bind(&AppNew::onMouseDown, this, std::placeholders::_1);
      dynamic_cast<OpenGLGraphicsDomain *>(domainPtr)->app.onMouseUp  = std::bind(&AppNew::onMouseUp, this, std::placeholders::_1);
      dynamic_cast<OpenGLGraphicsDomain *>(domainPtr)->app.onMouseDrag  = std::bind(&AppNew::onMouseDrag, this, std::placeholders::_1);
      dynamic_cast<OpenGLGraphicsDomain *>(domainPtr)->app.onMouseMove  = std::bind(&AppNew::onMouseMove, this, std::placeholders::_1);
      dynamic_cast<OpenGLGraphicsDomain *>(domainPtr)->app.onMouseScroll  = std::bind(&AppNew::onMouseScroll, this, std::placeholders::_1);

      mSimulationDomain->simulationFunction = std::bind(&AppNew::onAnimate, this, std::placeholders::_1);
    } else if (strcmp(typeid(*domainPtr).name(), typeid(AudioDomain).name()) == 0) {
      dynamic_cast<AudioDomain *>(domainPtr)->onSound = std::bind(&AppNew::onSound, this, std::placeholders::_1);
    } else if (strcmp(typeid(*domainPtr).name(), typeid(OSCDomain).name()) == 0) {
      dynamic_cast<OSCDomain *>(domainPtr)->onMessage = std::bind(&AppNew::onMessage, this, std::placeholders::_1);
    } else {
      std::cout << "WARNING: Domain unknown for auto connection" << std::endl;
    }
    if (!domain->initialize()) {
      std::cerr << "ERROR initializing domain " << std::endl;
    }
  }
}
