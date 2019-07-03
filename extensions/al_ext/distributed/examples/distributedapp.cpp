#include <memory>
#include <iostream>

#include "al_ext/distributed/al_App.hpp"

#include "al_ext/distributed/al_SimulationDomain.hpp"

#include "Gamma/Oscillator.h"

using namespace al;

struct State {
  float value;
};

class DistributedApp : public BaseCompositeApp {
public:
  DistributedApp(bool primary = true) : BaseCompositeApp() {

    // State will be same memory for local, but will be synced on the network for separate instances

    mOpenGLGraphicsDomain->removeSubDomain(simulationDomain());

    // Replace Simulation domain with state simulation domain
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
      mSimulationDomain->disableProcessingCallback(); // Replicas won't call onAnimate()
    }
  }

  State &state() {
    return std::static_pointer_cast<StateSimulationDomain<State>>(mSimulationDomain)->state();
  }

  void setTitle(std::string title) {
    graphicsDomain()->app.title(title);
  }

private:
  bool mPrimary;
};

class MyApp: public DistributedApp {
public:
  gam::Sine<> mOsc {440};
  gam::Sine<> mOsc2 {1};

  MyApp(bool primary = true) : DistributedApp(primary) {
    mOsc.domain(*audioDomain());
    mOsc2.domain(*graphicsDomain());

  }

  void onAnimate(double dt) {
    state().value = mOsc2();
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

};

int main(int argc, char *argv[])
{
  if (argc > 1 || !osc::Recv::portAvailable(9010, "0.0.0.0")) { // Run replica
    MyApp app(false);
    app.setTitle("REPLICA");
    app.start();

  } else {
    MyApp app;
    app.setTitle("PRIMARY");
    app.start();

  }

  return 0;
}
