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
  std::shared_ptr<StateReceiveDomain<State>> mReceiver;
  std::shared_ptr<StateSendDomain<State>> mSender;

  MyApp() {
    mOsc.domain(*audioDomain());
    mOsc2.domain(*graphicsDomain());

    mSender = graphicsDomain()->newSubDomain<StateSendDomain<State>>();
    mReceiver = graphicsDomain()->newSubDomain<StateReceiveDomain<State>>();

  }

  void onDraw(Graphics &g) override {
    // Update state
    mSender->state().value = mOsc2();
    // Use received values
    g.clear(0,0, mReceiver->state().value);
    std::cout << mReceiver->state().value << std::endl;
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
