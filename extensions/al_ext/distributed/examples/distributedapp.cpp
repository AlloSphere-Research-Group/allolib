#include <memory>
#include <iostream>

#include "al_ext/distributed/al_App.hpp"

#include "al_ext/distributed/al_StateDistributionDomain.hpp"

#include "Gamma/Oscillator.h"

using namespace al;

struct State {
  float value;
};

class MyApp: public BaseCompositeApp {
public:
  gam::Sine<> mOsc {440};
  gam::Sine<> mOsc2 {1};
  std::shared_ptr<StateDistributionDomain> mStateDistribution;
  std::shared_ptr<State> mState;

  MyApp() {
    mOsc.domain(*audioDomain());
    mOsc2.domain(*graphicsDomain());

    mStateDistribution = graphicsDomain()->newSubDomain<StateDistributionDomain>();
    // State will be same memory for local, but will be synced on the network for separate instances
    mState = mStateDistribution->addStateSender<State>();
    mStateDistribution->addStateReceiver<State>(mState);
  }

  void onDraw(Graphics &g) override {
    // Update state
    mState->value = mOsc2();
    // Use received values
    g.clear(0,0, mState->value);
    std::cout << mState->value << std::endl;
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
  MyApp app;
  app.start();


  return 0;
}
