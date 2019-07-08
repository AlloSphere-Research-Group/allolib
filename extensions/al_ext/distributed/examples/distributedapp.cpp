#include <memory>
#include <iostream>

#include "al_ext/distributed/al_App.hpp"

#include "al_ext/distributed/al_SimulationDomain.hpp"

#include "Gamma/Oscillator.h"

using namespace al;

struct State {
  float value;
};

class MyApp: public DistributedAppWithState<State> {
public:
  gam::Sine<> mOsc {440};
  gam::Sine<> mOsc2 {1};

  MyApp(uint16_t rank = 0) : DistributedAppWithState<State>(rank) {
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
    MyApp app(1);
    app.setTitle("REPLICA");
    app.start();

  } else {
    MyApp app(0);
    app.setTitle("PRIMARY");
    app.start();

  }

  return 0;
}
