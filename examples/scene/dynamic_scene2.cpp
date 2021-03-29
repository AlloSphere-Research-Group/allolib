#include <vector>

#include "Gamma/Oscillator.h"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"
#include "al/scene/al_DynamicScene.hpp"

// This example shows how to have "always on" voices in a DynamicScene

using namespace al;
using namespace std;
using namespace gam;

// We will have a global "Thing" object that will be shared by all voices in
// the scene. This is useful to avoid redundancy in the voices and
// conserve memory
struct Thing {
  Mesh mesh;
};

struct SimpleVoice : PositionedVoice {
  Sine<> oscillator;
  LFO<> lfo;  // 4 Hz Square wave for envelopeY

  float lfoValue{0.0f};

  virtual void onProcess(AudioIOData &io) override {
    while (io()) {
      lfoValue = lfo.downU();
      io.out(0) = oscillator() * lfoValue * 0.1f;
    }
  }

  virtual void onProcess(Graphics &g) override {
    Thing *thing = (Thing *)userData();
    g.color(lfoValue);
    g.draw(thing->mesh);
  }

  // Notice we haven't written onTriggerOn() and onTriggerOff() functions
  // As these voices are meant to be always on.
};

struct MyApp : App {
  DynamicScene scene{8};

  Thing thing;

  void onCreate() override {
    // Set thing to be the default user data for any allocated voices in the
    // scene
    scene.setDefaultUserData(&thing);
    //    // Don't forget to prepare the scene!
    //    scene.prepare(audioIO());

    // Prepare the mesh to be shared by all objects
    addCube(thing.mesh);

    // We will instantiate all visible objects at startup, i.e. they are always
    // on
    for (float i = 0.1f; i < 11; ++i) {
      // First allocate voice
      auto *freeVoice = scene.getVoice<SimpleVoice>();
      // Then configure voice
      freeVoice->oscillator.freq(330 * i);
      freeVoice->lfo.freq(0.3f + i * 0.25f);
      Pose p = freeVoice->pose();
      p.vec() = Vec3f(rnd::uniformS(4), rnd::uniformS(4), -3 - rnd::uniform(2));
      freeVoice->setPose(p);
      // The insert it in the rendering chain
      scene.triggerOn(freeVoice);
    }
  }

  void onAnimate(double /*dt*/) override {
    scene.listenerPose(nav());  // Set the scene listener to the current nav
  }

  void onDraw(Graphics &g) override {
    g.clear(0.2f);
    g.polygonLine();
    g.depthTesting(true);
    scene.render(g);
  }

  virtual void onSound(AudioIOData &io) override { scene.render(io); }
};

int main() {
  MyApp app;
  app.configureAudio();
  Domain::master().spu(app.audioIO().framesPerSecond());
  app.start();
}
