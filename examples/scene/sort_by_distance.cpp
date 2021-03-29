#include "Gamma/Oscillator.h"
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"
#include "al/scene/al_DynamicScene.hpp"

using namespace al;

struct MyVoice : public PositionedVoice {
  Mesh mesh;

  gam::Sine<> osc;

  Color c;

  void init() {
    addDisc(mesh);
    // Random freuqnecy of oscillation
    osc.freq(rnd::uniform(0.2, 0.05));

    // Set random color with alpha 0.2
    c = HSV(rnd::uniform(0.0, 1.0), 0.9, 0.9);
    c.a = 0.2;
  }

  void update(double dt) {
    auto newPose = mPose.get();
    newPose.pos().z = -6 + osc() * 3;
    mPose.set(newPose);
  }

  void onProcess(Graphics &g) {
    g.color(c);
    g.draw(mesh);
  }

  void onProcess(AudioIOData &io) {
    while (io()) {
      io.out(0) = 0.0;
    }
  }
};

struct MyApp : public App {
  DynamicScene scene;
  bool sortByDistance{true};

  void onCreate() {
    // Insert 8 voices in 'random' order.
    for (auto i = 0; i < 8; i++) {
      MyVoice *voice = scene.getVoice<MyVoice>();
      scene.triggerOn(voice);
    }
    gam::sampleRate(graphicsDomain()->fps());
  }

  void onAnimate(double dt) { scene.update(dt); }

  void onDraw(Graphics &g) {
    g.clear(0);
    g.blending(true);
    g.blendTrans();
    g.depthTesting(true);
    scene.render(g);
  }

  void onSound(AudioIOData &io) { scene.render(io); }

  bool onKeyDown(const Keyboard &k) {
    if (k.key() == ' ') {
      sortByDistance = !sortByDistance;

      scene.sortDrawingByDistance(sortByDistance);
      std::cout << "sorting set to " << sortByDistance << std::endl;
    }
    return true;
  }
};

int main() {
  MyApp().start();
  return 0;
}
