#include <iostream>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"

using namespace al;

struct MyApp : App {
  void onDraw(Graphics& g) override { g.clear(0, 0, 0); }

  void onResize(int w, int h) override {
    std::cout << "Resized to: " << w << "," << h << std::endl;
  }
};

int main() {
  MyApp app;
  app.dimensions(600, 400);
  app.start();
}
