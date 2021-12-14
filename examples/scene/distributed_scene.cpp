/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:
    Description:
*/

// This example shows how the DistributedScene class can be used
// to propagate changes in a DynamicScene across the network.
// Using DistributedApp together with DistributedScene, allows
// multiple instances of this same application binary to take
// on different roles. Try running two instances of this application
// on the same machine. The first will be the "primary" application
// controlling changes, and the second one will be a "replica"
// mirroring all the changes.

// See the examples for DynamicScene (dynamic_scene.cpp and
// avSequencer.cpp) for information on how it works.

#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"

#include "al/app/al_DistributedApp.hpp"
#include "al/math/al_Random.hpp"

#include "al/graphics/al_Shapes.hpp"
#include "al/io/al_AudioIO.hpp"
#include "al/scene/al_DistributedScene.hpp"

using namespace gam;
using namespace al;

// The Scene will contain "SimpleVoice" agents
class SimpleVoice : public PositionedVoice {
public:
  SimpleVoice() {
    mAmpEnv.levels(0, 1, 1, 0);
    mAmpEnv.lengths(2, 0.5, 2);
    addTorus(mMesh);
    mMesh.primitive(Mesh::LINE_STRIP);

    // Register mFreq and mAmp as continuous parameters of the voice
    registerParameters(mFreq, mAmp);

    // Change the oscillator's frequency whenever the parameter value changes
    mFreq.registerChangeCallback([this](float value) { mOsc.freq(value); });
  }

  virtual void update(double dt) override {
    if (!mIsReplica) {
      mFreq = mFreq * 0.992;
      mAmp = mAmpEnv.value();
    }
  }

  virtual void onProcess(AudioIOData &io) override {
    while (io()) {
      io.out(0) += mOsc() * mAmpEnv() * 0.1;
    }
    if (mAmpEnv.done())
      free();
  }

  virtual void onProcess(Graphics &g) override {
    g.color(mAmp);
    g.draw(mMesh);
  }

  virtual void onTriggerOn() override {
    mAmpEnv.reset();
    mFreq = 440;
    mAmp = 0;
  }

protected:
  Parameter mAmp{"Amp"};
  Parameter mFreq{"Freq"};

  Sine<> mOsc;
  Env<3> mAmpEnv;

  Mesh mMesh;
};

// make an app that contains a SynthSequencer class
// use the render() functions from the SynthSequencer to produce audio and
// graphics in the corresponding callback
struct MyApp : public DistributedApp {
  void onCreate() {
    //    parameterServer().print();

    scene.registerSynthClass<SimpleVoice>();
    registerDynamicScene(scene);
    scene.verbose(true);
  }

  void onAnimate(double dt) { scene.update(dt); }

  void onSound(AudioIOData &io) { scene.render(io); }

  void onDraw(Graphics &g) {
    g.clear();
    scene.render(g); // Render graphics
  }

  bool onKeyDown(Keyboard const &k) {
    if (isPrimary() && k.key() == ' ') { // Start a new voice on space bar
      auto *freeVoice = scene.getVoice<SimpleVoice>();
      Pose pose;
      pose.vec().x = al::rnd::uniform(2);
      pose.vec().y = al::rnd::uniform(2);
      pose.vec().z = -10.0 + al::rnd::uniform(6);
      freeVoice->setPose(pose);
      scene.triggerOn(freeVoice);
    }
    return true;
  }

  DistributedScene scene{TimeMasterMode::TIME_MASTER_CPU};
};

int main() {
  // Create app instance
  MyApp app;

  // Start everything
  app.configureAudio(48000., 1024, 2, 0);
  Domain::master().spu(app.audioIO().framesPerSecond());
  app.start();
}
