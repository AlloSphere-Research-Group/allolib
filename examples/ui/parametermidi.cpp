
#include <cmath>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ParameterMIDI.hpp"

using namespace al;

// This example shows how to connect MIDI CC messages to Parameter values

struct MyApp : App {
  float x = 0;
  Mesh m;

  Parameter Size{"Size", "", 1.0, "", 0, 1.0};
  Parameter Speed{"Speed", "", 0.05, "", 0.01, 0.3};

  ParameterMIDI parameterMIDI;

  void onInit() override {
    //    Connect parameters to MIDI CC messages
    parameterMIDI.connectControl(Size, 1, 1);
    parameterMIDI.connectControl(Speed, 10, 1);

    // Open MIDI device 0
    parameterMIDI.open(0);
  }

  void onCreate() override {
    addSphere(m);
    nav().pos(0, 0, 4);
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    g.pushMatrix();
    g.translate(std::sin(x), 0, 0);
    g.scale(Size.get());
    g.draw(m);
    g.popMatrix();
    x += Speed.get();
    if (x >= M_2PI) {
      x -= M_2PI;
    }
  }
};

int main() {
  MyApp().start();
  return 0;
}
