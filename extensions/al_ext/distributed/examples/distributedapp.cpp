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
      auto sender = std::static_pointer_cast<StateSimulationDomain<State>>(mSimulationDomain)->addStateSender("state");
      sender->configure(10101);
    } else {
      std::cout << "Running REPLICA" << std::endl;
      auto receiver = std::static_pointer_cast<StateSimulationDomain<State>>(mSimulationDomain)->addStateReceiver("state");
      receiver->configure(10101);
    }
  }

  void onAnimate(double dt) {
    if (mPrimary) {
      state().value = mOsc2();
    } else {
      std::cout << state().value << std::endl;
    }
  }

  void onDraw(Graphics &g) override {
    // Update state
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
  if (argc > 1 || !osc::Recv::portAvailable(9010, "0.0.0.0")) {
    MyApp app(false);
    app.start();

  } else {
    MyApp app;
    app.start();

  }

  return 0;
}
