#include <memory>
#include <iostream>

#include "al_ext/distributed/al_ComputationDomain.hpp"

#include "Gamma/Oscillator.h"

using namespace al;


class App: public BaseCompositeApp {
public:
  gam::Sine<> mOsc {440};
  gam::Sine<> mOsc2 {1};

  App() {
    mOsc.domain(*audioDomain());
    mOsc2.domain(*graphicsDomain());
  }

  void onDraw(Graphics &g) override {
    g.clear(0,0,mOsc2());
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
  App app;
  app.start();


  return 0;
}
