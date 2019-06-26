#include <memory>
#include <iostream>

#include "al_ext/distributed/al_App.hpp"

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

    static std::shared_ptr<OpenGLWindowDomain> wd;
    if (value > 2) {
      value -= 4.0f;
      if (!wd) {
        wd = graphicsDomain()->newWindow();
        wd->window().dimensions(400, 400, 100, 100);
        wd->window().title("Second window");
        wd->onDraw = std::bind(&MyApp::otherDraw, this, std::placeholders::_1);
      } else {
        graphicsDomain()->closeWindow(wd);
        wd = nullptr;
      }
    }
  }

  void onDraw(Graphics &g) override {
    g.clear(0,0,mOsc2());
    g.translate(0, value, -4);
    g.draw(mesh);
  }

  void otherDraw(Graphics &g) {
    g.clear(0,mOsc2.phase(), 0);
    g.translate(0, value, -4);
    g.draw(mesh);
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
