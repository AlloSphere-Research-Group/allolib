#include "al/core.hpp"

using namespace al;

struct MyApp : App
{

  void onCreate() override {

  }

  void onAnimate(double dt) override {
    
  }

  void onDraw(Graphics& g) override {
    g.clear(0, 0, 0);
  }
 
};

int main()
{
  MyApp app;
  app.dimensions(512, 512);
  app.start();
}