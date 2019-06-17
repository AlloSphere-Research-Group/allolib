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

  MyApp(bool primary = true) : BaseCompositeApp() {
    mOsc.domain(*audioDomain());
    mOsc2.domain(*graphicsDomain());

    mStateDistribution = graphicsDomain()->newSubDomain<StateDistributionDomain>();
    // State will be same memory for local, but will be synced on the network for separate instances 

    mPrimary = primary;
    if (mPrimary) {
      std::cout << "Running primary" << std::endl;
      mState = mStateDistribution->addStateSender<State>("state");
    } else {
      std::cout << "Running REPLICA" << std::endl;
      mState = mStateDistribution->addStateReceiver<State>("state");
      oscDomain()->configure(9100);
    }
  }

  void onDraw(Graphics &g) override {
    // Update state
    if (mPrimary) {
      mState->value = mOsc2();
    } else {
      std::cout << mState->value << std::endl;
    }
    // Use received values
    g.clear(0,0, mState->value);
  }

  void onSound(AudioIOData &io) override {
    while(io()) {
      io.out(0) =  mOsc() * 0.2;
    }
  }

  void onMessage(osc::Message &m) override {}

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
