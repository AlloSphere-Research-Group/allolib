#ifndef SIMULATIONDOMAIN_H
#define SIMULATIONDOMAIN_H

#include <stack>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>
#include <cassert>
#include <mutex>

#include "al_ComputationDomain.hpp"
#include "al_StateDistributionDomain.hpp"
#include "Gamma/Domain.h"

namespace al {

	/**
	 * @brief SimulationDomain class
	 * @ingroup App
	 */
class SimulationDomain: public SynchronousDomain {
public:

  virtual bool tick() override {
    bool ret = tickSubdomains(true);
    if (mUseCallback) {
      simulationFunction(timeDelta());
    }
    ret &= tickSubdomains(false);
    return true;
  }

  void disableProcessingCallback() { mUseCallback = false; }
  std::function<void(double dt)> simulationFunction = [](double){}; // function to be called in onAnimate()

private:
  bool mUseCallback {true};
};


template<class TSharedState>
class StateSimulationDomain : public SimulationDomain {
public:
  StateSimulationDomain() {
    mState = std::make_shared<TSharedState>();
  }

  virtual bool initialize(ComputationDomain *parent = nullptr) {
    SimulationDomain::initialize(parent);
    return true;
  }

  TSharedState &state() { return *mState;}

  std::shared_ptr<TSharedState> statePtr() { return mState;}


  std::shared_ptr<StateSendDomain<TSharedState>> addStateSender(std::string id = "") {
    auto newDomain = newSubDomain<StateSendDomain<TSharedState>>(false);
    newDomain->setId(id);
    newDomain->setStatePointer(statePtr());
    return newDomain;
  }

  std::shared_ptr<StateReceiveDomain<TSharedState>> addStateReceiver(std::string id = "") {
    auto newDomain = newSubDomain<StateReceiveDomain<TSharedState>>(true);
    newDomain->setId(id);
    newDomain->setStatePointer(statePtr());
    return newDomain;
  }


private:
  std::shared_ptr<TSharedState> mState;
};


} // namespace al


#endif //SIMULATIONDOMAIN
