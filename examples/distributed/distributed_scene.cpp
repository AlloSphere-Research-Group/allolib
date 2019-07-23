/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:
    Description:
*/

#include <cstdio>               // for printing to stdout
#define GAMMA_H_INC_ALL         // define this to include all header files
#define GAMMA_H_NO_IO           // define this to avoid bringing AudioIO from Gamma

#include "Gamma/Gamma.h"

#include "al/io/al_AudioIO.hpp"
#include "al/scene/al_DistributedScene.hpp"
#include "al/app/al_DistributedApp.hpp"
#include "al/graphics/al_Shapes.hpp"

using namespace gam;
using namespace al;

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


// The Scene will contain "SimpleVoice" agents
class SimpleVoice : public PositionedVoice {
public:
  SimpleVoice()
  {
    mAmpEnv.levels(0,1,1,0);
    mAmpEnv.lengths(2, 0.5, 2);

    // Register mFreq as the only parameter of the voice
    *this << mFreq << mEnvelope;

    // Change the oscillator's frequency whenever the parameter value changes
    mFreq.registerChangeCallback([this](float value) {
      mOsc.freq(value);
    });
  }

  virtual void update(double dt) override {
    auto p = pose();
    p.vec().y = mEnvelope;
    p.vec().x = mFreq/440.0;
    setPose(p);
  }

  virtual void onProcess(AudioIOData& io) override {
    while(io()){
      io.out(0) += mOsc() * mAmpEnv() * 0.1;
    }
    mEnvelope = mAmpEnv.value();
    if(mAmpEnv.done()) free();
  }

  virtual void onProcess(Graphics &g) override {
    g.color(mEnvelope);
    auto *mesh = (Mesh *) userData();
    g.draw(*mesh);
  }

  virtual void onTriggerOn() override {
    mAmpEnv.reset();
    mEnvelope = 0.0;
  }

  void updateFreq() {
    mFreq = mFreq * 0.992;
  }

protected:

  Parameter mAmp {"Amp"};
  Parameter mFreq {"Freq"};
  Parameter mDur {"Dur"};
  Parameter mEnvelope {"Dur"};

  Sine<> mOsc;
  Env<3> mAmpEnv;

};


// make an app that contains a SynthSequencer class
// use the render() functions from the SynthSequencer to produce audio and
// graphics in the corresponding callback
class MyApp : public DistributedApp<>
{
public:

  virtual void onCreate() override {

    scene.showWorldMarker(false);
    scene.registerSynthClass<SimpleVoice>();
    registerDynamicScene(scene);

    addTorus(mMesh);
    mMesh.primitive(Mesh::LINE_STRIP);
    mMesh.update();
    scene.setDefaultUserData((void *) &mMesh);

    if (isPrimary()) {
      title("Primary");

      // Trigger one voice manually
      auto *freeVoice = scene.getVoice<SimpleVoice>();
      std::vector<float> params{440.0f, 0.0f};
      freeVoice->setTriggerParams(params);
      auto p = freeVoice->pose();
      p.vec().z = -10.0;
      freeVoice->setPose(p);
      scene.triggerOn(freeVoice);
    } else {
      title("Replica");
    }
    parameterServer().print();
  }

  virtual void onAnimate(double dt) override {
    if (isPrimary()) {
      // Only primary node updates frequency. The replicas get notified
      auto *voice = scene.getActiveVoices();
      while (voice) {
        static_cast<SimpleVoice *>(voice)->updateFreq();
        voice = voice->next;
      }
    }
    scene.update(dt);
  }

  virtual void onSound(AudioIOData &io) override {
    scene.render(io); // Render audio
  }

  virtual void onDraw(Graphics &g) override {
    g.clear();
    scene.render(g); // Render graphics
  }

  //    virtual void onMessage(osc::Message &m) override {
  //        scene.consumeMessage(m);
  //    }

  virtual void onKeyDown(Keyboard const &k) override {
    if (k.key() == ' ') {
      auto *freeVoice = scene.getVoice<SimpleVoice>();
      std::vector<float> params{440.0f, 0.0f};
      freeVoice->setTriggerParams(params);
      auto p = freeVoice->pose();
      p.vec().z = -10.0;
      freeVoice->setPose(p);
      scene.triggerOn(freeVoice);
    }
  }

  DistributedScene scene{PolySynth::TIME_MASTER_CPU};
  float size {0.5f};

  VAOMesh mMesh; // This will be shared user data for all voices
};

int main(){
  // Create app instance
  MyApp app;
  app.initAudio(48000., 1024, 2,0);
  Domain::master().spu(app.audioIO().framesPerSecond());
  app.start();
}
