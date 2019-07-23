#ifdef AL_USE_CUTTLEBONE
#include "al/app/al_App.hpp"
#include "al/sphere/al_OmniRenderer.hpp"
#include "Cuttlebone/Cuttlebone.hpp"

#include "common.hpp"

using namespace std;
using namespace al;

struct MyRendererApp : OmniRenderer
{
  cuttlebone::Taker<State, 9000> taker;
  unique_ptr<State> state;

  Mesh mesh;
  
  void onCreate() override {
    state = make_unique<State>();
    taker.start();
    addIcosahedron(mesh);
  }

  void onAnimate(double dt) override {
    int popCount = taker.get(*state);
    pose(state->pose);
  }

  void onDraw(Graphics& g) override {
    g.clear(state->backgroundColor);
    g.color(0.5);
    g.draw(mesh);
  }

  void onExit() override {
    taker.stop();
  }

};

int main()
{
  MyRendererApp app;
  app.start();
}

# else
#include <iostream>

int main()
{
    std::cout << "Cuttlebone not available!!" << std::endl;
}

#endif