#include "al/app/al_SimulationDomain.hpp"

using namespace al;

bool SimulationDomain::tick() {
  bool ret = tickSubdomains(true);
  if (mUseCallback) {
    simulationFunction(timeDelta());
  }
  ret &= tickSubdomains(false);
  return true;
}

void SimulationDomain::disableProcessingCallback() { mUseCallback = false; }
