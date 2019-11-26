#include <cstdlib>
#include <iostream>
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/system/al_Signal.hpp"

/*

  Example file for showcasing how to utilize al_Signal.hpp

  ** TODO: replace contents of onexit with signal safe instructions

  Kon Hyong Kim, 2019
  konhyong@gmail.com

*/

using namespace al;
using namespace std;

struct MyApp : App {
  Mesh mesh;
  double phase = 0;

  short port = 9010;
  const char* addr = "127.0.0.1";
  osc::Send client;

  void onCreate() override {
    addTetrahedron(mesh);
    lens().near(0.1).far(25).fovy(45);
    nav().pos(0, 0, 4);
    nav().quat().fromAxisAngle(0. * M_2PI, 0, 1, 0);

    client.open(port, addr);

    registerSigInt(this);  // *** need this line to register signal handler
  }

  void onExit() override {
    std::cout << "onExit!!!!" << std::endl;

    exitExtra();
  }

  void exitExtra() {
    std::cout << "doing extra stuff" << std::endl;
    client.send("/test", "handshakeoff", 1);
  }

  void onAnimate(double dt) override {
    double period = 10;
    phase += dt / period;
    if (phase >= 1.) phase -= 1.;
  }

  void onDraw(Graphics& g) override {
    g.clear(0, 0, 0);
    g.polygonMode(Graphics::LINE);
    g.pushMatrix();
    g.rotate(phase * 360, 0, 1, 0);
    g.color(1);
    g.draw(mesh);
    g.popMatrix();
  }

  bool onKeyDown(Keyboard const& k) override {
    if (k.key() == ' ') {
      std::cout << "space" << std::endl;
      while (1)
        ;
    }
  }

  void onMessage(osc::Message& m) override { m.print(); }
};

int main() {
  MyApp app;

  // registerSigInt(&app);  // alternate location for registerSigInt

  app.dimensions(600, 400);
  app.start();
}