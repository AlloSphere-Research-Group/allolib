#ifndef AL_WINDOWS
#include "al/core.hpp"
#include "al/sphere/al_SphereUtils.hpp"
#include "Cuttlebone/Cuttlebone.hpp"

#include "common.hpp"

using namespace std;
using namespace al;

const char* defaultBroadcastIP() {
  if(sphere::is_in_sphere()) return "192.168.10.255";
  else return "127.0.0.1";
}

struct MySimulatorApp : App
{
  cuttlebone::Maker<State, 9000> maker {defaultBroadcastIP()};
  unique_ptr<State> state;
  Mesh mesh;
    
  void onCreate() override {
    state = make_unique<State>();
    maker.start();
    nav().pos(0, 0, 5);
    nav().faceToward({0, 0, 0}, {0, 1, 0});
    addIcosahedron(mesh);
  }

  void onAnimate(double dt) override {
    state->backgroundColor.r = float(mouse().x()) / width();
    state->backgroundColor.g = float(mouse().y()) / height();
    state->pose = pose();
    maker.set(*state);
  }

  void onDraw(Graphics& g) override {
    g.clear(state->backgroundColor);
    g.color(0.5);
    g.draw(mesh);
  }

  void onExit() override {
    maker.stop();
  }

};

int main()
{
  MySimulatorApp app;
  app.start();
}

# else

#include <iostream>

int main()
{
    std::cout << "Not implemented on Windows!!" << std::endl;
}

#endif