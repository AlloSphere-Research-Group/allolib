
#include <iostream>

#include "al/app/al_DistributedApp.hpp"
#include "al/graphics/al_Image.hpp"
//#include "al/app/al_OmniRendererDomain.hpp"

using namespace std;
using namespace al;

struct MyOmniRendererApp : DistributedApp {
  VAOMesh mesh;
  ParameterPose currentPose{"currentPose"};
  Texture texture;

  bool DO_BLENDING = false;
  float alpha = 0.9f;

  void onCreate() override {
    //    append(mNavControl);
    addIcosahedron(mesh);
    mesh.update();
    parameterServer() << currentPose;

    // Load texture
    const char *filename = "../../graphics/bin/data/hubble.jpg";

    auto imageData = Image(filename);

    if (imageData.array().size() == 0) {
      cout << "failed to load image " << filename << endl;
    } else {
      cout << "loaded image size: " << imageData.width() << ", "
           << imageData.height() << endl;
      texture.create2D(imageData.width(), imageData.height());
      texture.submit(imageData.array().data(), GL_RGBA, GL_UNSIGNED_BYTE);

      texture.filter(Texture::LINEAR);
    }
  }

  void onAnimate(double dt) override {
    if (isPrimary()) {
      currentPose = pose();
    } else {
      pose().set(currentPose.get());
    }
  }

  void onDraw(Graphics &g) override {
    g.clear(0, 0, 1);
    {
      g.pushMatrix();
      g.translate(0, 0, -2);
      g.quad(texture, -0.5, -0.5, 1, 1);
      g.popMatrix();
    }
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
          if (aa == 0 && bb == 0 && cc == 0)
            continue;
          g.pushMatrix();
          g.translate(aa * 2, bb * 2, cc * 2);
          //            g.rotate(sin(2 * al::seconds()), 0, 0, 1);
          //            g.rotate(sin(3 * sec()), 0, 1, 0);
          g.scale(0.2, 0.2, 0.2);
          if (DO_BLENDING) {
            g.color((aa + 5) / 10.0, (bb + 5) / 10.0, (cc + 5) / 10.0, alpha);
          } else {
            g.color((aa + 5) / 10.0, (bb + 5) / 10.0, (cc + 5) / 10.0);
          }
          g.draw(mesh);
          g.popMatrix();
        }
  }

  bool onKeyDown(const Keyboard &k) override {
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
    return false;
  }
};

int main() {
  MyOmniRendererApp app;
  app.start();
}
