#include <memory>
#include <iostream>

#include "al_ext/distributed/al_App.hpp"

#include "al_ext/distributed/al_SimulationDomain.hpp"

#include "Gamma/Oscillator.h"

using namespace al;

struct State {
  float value;
};

class MyApp: public BaseCompositeApp {
public:
  gam::Sine<> mOsc {440};
  gam::Sine<> mOsc2 {1};

  MyApp(bool primary = true) : BaseCompositeApp() {
    mOsc.domain(*audioDomain());
    mOsc2.domain(*graphicsDomain());

    // State will be same memory for local, but will be synced on the network for separate instances 

    mOpenGLGraphicsDomain->removeSubDomain(simulationDomain());
    mSimulationDomain = mOpenGLGraphicsDomain->newSubDomain<StateSimulationDomain<State>>(true);

    mPrimary = primary;
    if (mPrimary) {
      std::cout << "Running primary" << std::endl;
    } else {
      std::cout << "Running REPLICA" << std::endl;
      std::static_pointer_cast<StateSimulationDomain<State>>(mSimulationDomain)->addStateReceiver("state");
      oscDomain()->configure(9100);
    }
  }

  void onDraw(Graphics &g) override {
    // Update state
    if (mPrimary) {
      state().value = mOsc2();
    } else {
      std::cout << state().value << std::endl;
    }
    // Use received values
    g.clear(0,0, state().value);
  }

  void onSound(AudioIOData &io) override {
    while(io()) {
      io.out(0) =  mOsc() * 0.2;
    }
  }

  void onMessage(osc::Message &m) override {}

  State &state() {
    return std::static_pointer_cast<StateSimulationDomain<State>>(mSimulationDomain)->state();
  }

private:
  bool mPrimary;
};

int main(int argc, char *argv[])
{
  if (argc > 1) {
    MyApp app(false);
    app.start();

  } else {
    MyApp app;
    app.start();

  }


  return 0;
}
