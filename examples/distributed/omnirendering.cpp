
#include <iostream>

#include "al/app/al_DistributedApp.hpp"
//#include "al/app/al_OmniRendererDomain.hpp"

using namespace std;
using namespace al;

struct MyOmniRendererApp : DistributedApp {
  VAOMesh mesh;
  ParameterPose currentPose{"currentPose"};

  bool DO_BLENDING = false;
  float alpha = 0.9f;

  void onCreate() override {
    //    append(mNavControl);
    addIcosahedron(mesh);
    mesh.update();
    parameterServer() << currentPose;
  }

  void onAnimate(double dt) override {
    if (isPrimary()) {
      currentPose = pose();
    } else {
      pose().set(currentPose.get());
    }
  }

  void onDraw(Graphics& g) override {
    g.clear(0, 0, 1);

    if (DO_BLENDING) {
      g.depthTesting(false);
      g.blending(true);
      g.blendAdd();
    } else {
      g.depthTesting(true);
      g.depthMask(true);
      g.blending(false);
    }

    for (int aa = -5; aa <= 5; aa++)
      for (int bb = -5; bb <= 5; bb++)
        for (int cc = -5; cc <= 5; cc++) {
          if (aa == 0 && bb == 0 && cc == 0) continue;
          g.pushMatrix();
          g.translate(aa * 2, bb * 2, cc * 2);
          //            g.rotate(sin(2 * al::seconds()), 0, 0, 1);
          //            g.rotate(sin(3 * sec()), 0, 1, 0);
          g.scale(0.3, 0.3, 0.3);
          if (DO_BLENDING) {
            g.color((aa + 5) / 10.0, (bb + 5) / 10.0, (cc + 5) / 10.0, alpha);
          } else {
            g.color((aa + 5) / 10.0, (bb + 5) / 10.0, (cc + 5) / 10.0);
          }
          g.draw(mesh);
          g.popMatrix();
        }
  }

  bool onKeyDown(const Keyboard& k) override {
    if (k.key() == 'b') {
      DO_BLENDING = !DO_BLENDING;
      cout << "blending: " << DO_BLENDING << endl;
    }
    if (k.key() == 'n') {
      alpha = 1 - alpha;
      cout << "alpha: " << alpha << endl;
    }
    if (k.key() == 'o') {
      omniRendering->drawOmni = !omniRendering->drawOmni;
    }
    return true;
  }
};

int main() {
  MyOmniRendererApp app;
  app.start();
}
