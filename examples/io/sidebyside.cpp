#include <iostream>
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"

using namespace al;

struct MyApp : App {
  Mesh mesh;
  double phase = 0;

  void onCreate() override {
    addTetrahedron(mesh);
    lens().near(0.1).far(25).fovy(45);
    nav().pos(0, 0, 4);
    nav().quat().fromAxisAngle(0. * M_2PI, 0, 1, 0);
  }

  void onAnimate(double dt) override {
    double period = 10000;
    phase += dt / period;
    if (phase >= 1.) phase -= 1.;
  }

  void onDraw(Graphics& g) override {
    /*  Left
     */
    g.viewport(0, 0, fbWidth() / 2, fbHeight());
    g.scissorTest(true);
    g.scissorArea(0, 0, fbWidth() / 2, fbHeight());

    g.clear(1, 0, 0);
    g.polygonLine();
    g.pushMatrix();
    g.rotate(phase * 360, 0, 1, 0);
    g.color(1);
    g.draw(mesh);
    g.popMatrix();

    /*  Right
     */
    g.viewport(fbWidth() / 2, 0, fbWidth() / 2, fbHeight());
    g.scissorTest(true);
    g.scissorArea(fbWidth() / 2, 0, fbWidth() / 2, fbHeight());

    g.clear(0, 0, 1);
    g.polygonLine();
    g.pushMatrix();
    g.rotate(phase * 360, 0, 1, 0);
    g.color(1);
    g.draw(mesh);
    g.popMatrix();

    /*  Reset settings
     */
    g.viewport(0, 0, fbWidth(), fbHeight());
    g.scissorTest(false);
  }
};

int main() {
  MyApp app;
  app.dimensions(600, 400);
  app.decorated(false);
  app.start();
}
