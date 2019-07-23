
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"

#include "al/ui/al_ParameterMIDI.hpp"

#include <cmath>

using namespace al;


struct MyApp : App
{
  float x = 0;
  Mesh m;

  Parameter Size{"Size", "", 1.0, "", 0, 1.0};
  Parameter Speed{"Speed", "", 0.05, "", 0.01, 0.3};

  ParameterMIDI parameterMIDI;

  void onInit() override {
    parameterMIDI.connectControl(Size, 1, 1);
    parameterMIDI.connectControl(Speed, 10, 1);
  }

  void onCreate() override {
    addSphere(m);
    nav().pos(0, 0, 4);
  }

  void onDraw(Graphics& g) override {
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

int main(int argc, char* argv[])
{
  MyApp().start();
}

