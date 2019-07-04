#ifndef BASECOMPOSITEAPP_H
#define BASECOMPOSITEAPP_H

#include <stack>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>
#include <cassert>
#include <cstring>

#include "al_ComputationDomain.hpp"
#include "al_OpenGLGraphicsDomain.hpp"
#include "al_AudioDomain.hpp"
#include "al_OSCDomain.hpp"
#include "al_SimulationDomain.hpp"
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

    mOpenGLGraphicsDomain = newDomain<OpenGLGraphicsDomain>();
    mSimulationDomain = mOpenGLGraphicsDomain->newSubDomain<SimulationDomain>(true);
  }


  template<class DomainType>
  std::shared_ptr<DomainType> newDomain() {
    auto newDomain = std::make_shared<DomainType>();
    mDomainList.push_back(newDomain);
    if (!newDomain->initialize()) {
      std::cerr << "ERROR initializing domain " << std::endl;
    }
    return newDomain;
  }

  virtual void onInit () {}
  virtual void onCreate() {}
  virtual void onAnimate(double dt) {(void) dt;}
  virtual void onDraw(Graphics &g) { (void) g;}
  virtual void onSound(AudioIOData &io) { (void) io;}
  virtual void onMessage(osc::Message &m) { (void) m;}
  virtual void onExit() {}

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
  std::shared_ptr<OpenGLGraphicsDomain> graphicsDomain() { return mOpenGLGraphicsDomain;}
  std::shared_ptr<SimulationDomain> simulationDomain() { return mSimulationDomain;}

  void enableVR() {

#ifdef AL_EXT_OPENVR
    if (!mOpenVRDomain) {
      std::cout << "Attempting to create OpenVRDomain" << std::endl;
      mOpenVRDomain = graphicsDomain()->newSubDomain<OpenVRDomain>(true);
    }
#else
    std::cout << "OpenVR support not available. Ignoring enableVR()" << std::endl;
#endif
  }

  void disableVR() {

#ifdef AL_EXT_OPENVR
    graphicsDomain()->removeSubDomain(mOpenVRDomain);
    mOpenVRDomain = nullptr;
#else
    std::cout << "OpenVR support not available. Ignoring enableVR()" << std::endl;
#endif
  }

protected:

  void initializeDomains();

  std::shared_ptr<OSCDomain> mOSCDomain;
  std::shared_ptr<AudioDomain> mAudioDomain;
  std::shared_ptr<OpenGLGraphicsDomain> mOpenGLGraphicsDomain;
  std::shared_ptr<SimulationDomain> mSimulationDomain;
  std::shared_ptr<OpenVRDomain> mOpenVRDomain;


  std::vector<std::shared_ptr<AsynchronousDomain>> mDomainList;
  std::stack<std::shared_ptr<AsynchronousDomain>> mRunningDomains;
};


class DistributedApp : public BaseCompositeApp {
public:
  DistributedApp(bool primary = true) : BaseCompositeApp() {

    // State will be same memory for local, but will be synced on the network for separate instances

    mOpenGLGraphicsDomain->removeSubDomain(simulationDomain());

    // Replace Simulation domain with state simulation domain
    mSimulationDomain = mOpenGLGraphicsDomain->newSubDomain<SimulationDomain>(true);

    mPrimary = primary;
    if (mPrimary) {
      std::cout << "Running Primary" << std::endl;

    } else {
      std::cout << "Running REPLICA" << std::endl;

      mSimulationDomain->disableProcessingCallback(); // Replicas won't call onAnimate()
    }
  }


  void setTitle(std::string title) {
    graphicsDomain()->app.title(title);
  }

private:
  bool mPrimary;
};

template <class TSharedState>
class DistributedAppWithState : public DistributedApp {
public:
  DistributedAppWithState(uint16_t rank_) : DistributedApp(), rank(rank_) {

    // State will be same memory for local, but will be synced on the network for separate instances

    mOpenGLGraphicsDomain->removeSubDomain(simulationDomain());

    // Replace Simulation domain with state simulation domain
    mSimulationDomain = mOpenGLGraphicsDomain->newSubDomain<StateSimulationDomain<TSharedState>>(true);

    if (rank == 0) {
      std::cout << "Running primary" << std::endl;
      auto sender = std::static_pointer_cast<StateSimulationDomain<TSharedState>>(mSimulationDomain)->addStateSender("state");
      sender->configure(10101);
    } else {
      std::cout << "Running REPLICA" << std::endl;
      auto receiver = std::static_pointer_cast<StateSimulationDomain<TSharedState>>(mSimulationDomain)->addStateReceiver("state");
      receiver->configure(10101);
      mSimulationDomain->disableProcessingCallback(); // Replicas won't call onAnimate()
    }
  }

  TSharedState &state() {
    return std::static_pointer_cast<StateSimulationDomain<TSharedState>>(mSimulationDomain)->state();
  }

  void setTitle(std::string title) {
    graphicsDomain()->app.title(title);
  }

  uint16_t rank {0};
  uint16_t group {0};
private:
};


} // namespace al

#endif // BASECOMPOSITEAPP_H
