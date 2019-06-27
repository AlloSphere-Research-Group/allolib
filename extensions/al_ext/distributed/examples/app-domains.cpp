#include <memory>
#include <iostream>

#include "al_ext/distributed/al_App.hpp"
#include "al/core/graphics/al_Shapes.hpp"

#include "Gamma/Oscillator.h"

using namespace al;


class MyApp: public BaseCompositeApp {
public:
  gam::Sine<> mOsc {440};
  gam::Sine<> mOsc2 {1};

  float value = 0.0; // Value computed in onAnimate

  Mesh mesh;

  MyApp() {
    mOsc.domain(*audioDomain());
    mOsc2.domain(*graphicsDomain());
  }

  void onInit() override {
    addWireBox(mesh);
  }

  void onAnimate(double dt) override {
    value += dt;
  }

  void onDraw(Graphics &g) override {
    g.clear(0,0,mOsc2());
    g.translate(0, value, -4);
    g.draw(mesh);
  }

  void onSound(AudioIOData &io) override {
    while(io()) {
      io.out(0) =  mOsc() * 0.2;
    }
  }

  void onMessage(osc::Message &m) override {
    m.print();
  }

};

int main(int argc, char *argv[])
{
  MyApp app;
  app.start();


  return 0;
}
