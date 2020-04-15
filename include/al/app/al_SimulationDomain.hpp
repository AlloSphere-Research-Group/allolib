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

namespace al {

/**
 * @brief Basic simulation domain providing a simulationFunction
 * @ingroup App
 *
 */
class SimulationDomain : public SynchronousDomain {
public:
  virtual bool tick() override;

  void disableProcessingCallback();
  std::function<void(double dt)> simulationFunction = [](double) {
  }; // function to be called in onAnimate()

private:
  bool mUseCallback{true};
};

// -------------

/**
 * Simulation domain with state
 */
template <class TSharedState>
class StateSimulationDomain : public SimulationDomain {
public:
  TSharedState &state() { return *mState; }

  std::shared_ptr<TSharedState> statePtr() { return mState; }

protected:
  std::shared_ptr<TSharedState> mState{new TSharedState};
};

} // namespace al

#endif // SIMULATIONDOMAIN
