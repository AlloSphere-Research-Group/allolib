#ifndef BASECOMPOSITEAPP_H
#define BASECOMPOSITEAPP_H

#include <stack>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>
#include <cassert>

#include "al_ComputationDomain.hpp"
#include "al_GraphicsDomain.hpp"
#include "al_AudioDomain.hpp"
#include "al_OSCDomain.hpp"
#include "al_OpenVRDomain.hpp"

#include "al/core/app/al_WindowApp.hpp"

namespace  al
{

class BaseCompositeApp {
public:

  BaseCompositeApp() {
    mOSCDomain = newDomain<OSCDomain>();

    mAudioDomain = newDomain<AudioDomain>();
    mAudioDomain->configure();

    mGraphicsDomain = newDomain<GraphicsDomain>();
#ifdef AL_EXT_OPENVR
    mGraphicsDomain->newSubDomain<OpenVRDomain>();
#endif
  }


  template<class DomainType>
  std::shared_ptr<DomainType> newDomain() {
    auto newDomain = std::make_shared<DomainType>();
    mDomainList.push_back(newDomain);
    return newDomain;
  }

  virtual void onInit () {}
  virtual void onCreate() {}
  virtual void onDraw(Graphics &g) { (void) g;}
  virtual void onSound(AudioIOData &io) { (void) io;}
  virtual void onMessage(osc::Message &m) { (void) m;}

  void setOpenVRDrawFunction(std::function<void(Graphics &)> func) {
#ifdef AL_EXT_OPENVR
    mOpenVRDomain->setDrawFunction(func);
#else
    std::cout << "Not building OpenVR support. setOpenVRDrawFunction() ignored." << std::endl;
#endif
  }

  void start();

  std::shared_ptr<OSCDomain> oscDomain() {return mOSCDomain;}
  std::shared_ptr<AudioDomain> audioDomain() { return mAudioDomain;}
  std::shared_ptr<GraphicsDomain> graphicsDomain() { return mGraphicsDomain;}

private:

  std::shared_ptr<OSCDomain> mOSCDomain;
  std::shared_ptr<AudioDomain> mAudioDomain;
  std::shared_ptr<GraphicsDomain> mGraphicsDomain;
  std::shared_ptr<OpenVRDomain> mOpenVRDomain;

  std::vector<std::shared_ptr<AsynchronousDomain>> mDomainList;
  std::stack<std::shared_ptr<AsynchronousDomain>> mRunningDomains;
};

} // namespace al

#endif // BASECOMPOSITEAPP_H
