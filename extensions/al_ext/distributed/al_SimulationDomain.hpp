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

class SimulationDomain: public SynchronousDomain {
public:

  virtual bool tick() override {
    bool ret = tickSubdomains(true);
    simulationFunction(timeDelta());
    ret &= tickSubdomains(false);
    return true;
  }

  std::function<void(double dt)> simulationFunction = [](double){}; // function to be called in onAnimate()
};


template<class TSharedState>
class StateSimulationDomain : public SimulationDomain {
public:

  virtual bool initialize(ComputationDomain *parent = nullptr) {
    mState = std::make_shared<TSharedState>();
    SimulationDomain::initialize(parent);
    return true;
  }

  TSharedState &state() { return *mState;}

  template<class TSharedState>
  std::shared_ptr<StateSendDomain<TSharedState>> addStateSender(std::string id = "") {
    auto newDomain = newSubDomain<StateSendDomain<TSharedState>>(this);
    newDomain->setId(id);
    newDomain->setStatePointer(mState);
    return newDomain;
  }

  template<class TSharedState = DefaultState>
  std::shared_ptr<StateReceiveDomain<TSharedState>> addStateReceiver(std::string id = "") {
    auto newDomain = newSubDomain<StateReceiveDomain<TSharedState>>(this);
    newDomain->setId(id);
    newDomain->setStatePointer(mState);
    return newDomain;
  }


private:
  std::shared_ptr<TSharedState> mState;
};


} // namespace al


#endif //SIMULATIONDOMAIN
