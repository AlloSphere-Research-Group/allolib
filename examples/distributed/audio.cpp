
#include <iostream>

#include "al/app/al_DistributedApp.hpp"
#include "al/math/al_Random.hpp"

using namespace std;
using namespace al;

struct MyApp : DistributedApp {
  VAOMesh mesh;

  void onCreate() override {
    addIcosahedron(mesh);
    mesh.update();
  }

  void onInit() override { audioDomain()->audioIO().print(); }

  void onSound(AudioIOData &io) override {
    while (io()) {
      io.out(0) = rnd::uniform(-0.1, 0.1);
    }
  }

  void onDraw(Graphics &g) override {
    g.clear(0, 0, 1);

    for (int aa = -5; aa <= 5; aa++)
      for (int bb = -5; bb <= 5; bb++)
        for (int cc = -5; cc <= 5; cc++) {
          if (aa == 0 && bb == 0 && cc == 0)
            continue;
          g.pushMatrix();
          g.translate(aa * 2, bb * 2, cc * 2);
          g.scale(0.2, 0.2, 0.2);

          g.color((aa + 5) / 10.0, (bb + 5) / 10.0, (cc + 5) / 10.0);
          g.draw(mesh);
          g.popMatrix();
        }
  }
};

int main() {
  MyApp app;
  app.start();
}
