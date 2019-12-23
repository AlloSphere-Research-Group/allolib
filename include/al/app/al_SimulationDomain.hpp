#ifndef SIMULATIONDOMAIN_H
#define SIMULATIONDOMAIN_H

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <stack>
#include <vector>

#include "al/app/al_ComputationDomain.hpp"
#include "al/app/al_StateDistributionDomain.hpp"

namespace al {

/**
 * @brief SimulationDomain class
 * @ingroup App
 */
class SimulationDomain : public SynchronousDomain {
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
  std::function<void(double dt)> simulationFunction = [](double) {
  };  // function to be called in onAnimate()

 private:
  bool mUseCallback{true};
};

template <class TSharedState>
class StateSimulationDomain : public SimulationDomain {
 public:
  TSharedState &state() { return *mState; }

  std::shared_ptr<TSharedState> statePtr() { return mState; }

  //  virtual std::shared_ptr<StateSendDomain<TSharedState>> addStateSender(
  //      std::string id = "") {
  //    auto newDomain = newSubDomain<StateSendDomain<TSharedState>>(false);
  //    newDomain->setId(id);
  //    newDomain->setStatePointer(statePtr());
  //    return newDomain;
  //  }

  //  virtual std::shared_ptr<StateReceiveDomain<TSharedState>>
  //  addStateReceiver(
  //      std::string id = "") {
  //    auto newDomain = newSubDomain<StateReceiveDomain<TSharedState>>(true);
  //    newDomain->setId(id);
  //    newDomain->setStatePointer(statePtr());
  //    return newDomain;
  //  }

 protected:
  std::shared_ptr<TSharedState> mState{new TSharedState};
};

}  // namespace al

#endif  // SIMULATIONDOMAIN
