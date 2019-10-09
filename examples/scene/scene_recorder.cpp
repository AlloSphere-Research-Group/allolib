/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:
    Description:
*/

#include <cstdio>        // for printing to stdout
#define GAMMA_H_INC_ALL  // define this to include all header files
#define GAMMA_H_NO_IO    // define this to avoid bringing AudioIO from Gamma

#include "Gamma/Gamma.h"

//#include "al/io/al_AudioIO.hpp"
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/scene/al_DynamicScene.hpp"
#include "al/scene/al_SynthRecorder.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_ControlGUI.hpp"

using namespace gam;
using namespace al;

// A DynamicScene manages the insertion and removal of
// PositionedVoice nodes in its rendering graph.
// A DynamicScene has three rendering contexts:
// The update() or simulation context, where state and
// internal changes should be computed, and the onProcess()
// contexts for audio and graphics.

// The DynamicScene will contain "SimpleVoice" agents that
// inherit from PositionedVoice
class SimpleVoice : public PositionedVoice {
 public:
  SimpleVoice() {
    mAmpEnv.sustainPoint(1);
    addTorus(mMesh);
    mMesh.primitive(Mesh::LINE_STRIP);

    // Register parameters using the stream operator
    // parameters registered this way can be set through
    // the setTriggerParams(), and will also allow their
    // values to be stored when using SynthSequencer
    *this << mFreq << mText;

    // The Freq parameter will drive changes in the
    // internal oscillator
    mFreq.registerChangeCallback([this](float value) { mOsc.freq(value); });
  }

  // The update function will change the position of the
  virtual void update(double /*dt*/) override {
    mFreq = mFreq * 0.995f;
    Pose p = pose();
    p.vec().y = double(mAmpEnv.value() * 3.0f);
    p.vec().x = double(mFreq / 440.0f);
    setPose(p);
  }

  virtual void onProcess(AudioIOData &io) override {
    while (io()) {
      io.out(0) += mOsc() * mAmpEnv() * 0.05f;
    }
    if (mAmpEnv.done()) {
      std::cout << "free" << std::endl;
      free();
    }
  }

  virtual void onProcess(Graphics &g) override {
    HSV c;
    c.h = mAmpEnv.value();
    g.color(Color(c));
    g.draw(mMesh);
  }

  virtual void onTriggerOn() override {
    Pose p = pose();
    p.vec() = {double(mFreq / 440.0f), 0.0, -10.0};
    setPose(p);
    mAmpEnv.reset();
  }
  virtual void onTriggerOff() override { mAmpEnv.release(); }

 protected:
  Parameter mFreq{"Freq"};
  ParameterString mText{"Text"};

  Sine<> mOsc;
  AD<> mAmpEnv{3.0f, 3.0f};

  Mesh mMesh;
};

// make an app that contains a SynthSequencer class
// use the render() functions from the SynthSequencer to produce audio and
// graphics in the corresponding callback
class MyApp : public App {
 public:
  virtual void onCreate() override {
    navControl().disable();
    scene.registerSynthClass<SimpleVoice>();
    // Preallocate 300 voices
    scene.allocatePolyphony("SimpleVoice", 30);
    //        scene.prepare(audioIO());
    gui.init();
    gui << recorder;
    gui << sequencer;
    recorder << scene;
    sequencer << scene;
  }

  virtual void onAnimate(double dt) override {
    scene.update(dt);  // Update all nodes in the scene
  }

  virtual void onSound(AudioIOData &io) override {
    sequencer.render(io);  // Render audio
  }

  virtual void onDraw(Graphics &g) override {
    //        s.print(); // Prints information on active and free voices
    g.clear();
    // Render scene on the left
    g.pushMatrix();
    sequencer.render(g);  // Render graphics
    g.popMatrix();
    gui.draw(g);
  }

  bool onKeyDown(const Keyboard &k) override {
    // Trigger one new voice every 0.05 seconds
    // First get a free voice of type SimpleVoice
    auto *freeVoice = scene.getVoice<SimpleVoice>();
    std::string text = std::to_string(char(k.key()));
    freeVoice->setTriggerParams({440.f, text});
    // Set a position for it
    // Trigger it (this inserts it into the chain)
    scene.triggerOn(freeVoice, 0, k.key());
    return true;
  }

  bool onKeyUp(const Keyboard &k) override {
    scene.triggerOff(k.key());
    return true;
  }

  // The number passed to the construtor indicates how many threads are used to
  // compute the scene
  DynamicScene scene{4};
  SynthRecorder recorder;
  SynthSequencer sequencer;

  ControlGUI gui;
};

int main() {
  // Create app instance
  MyApp app;

  // Start everything
  app.configureAudio(44100., 256, 2, 2);
  Domain::master().spu(app.audioIO().framesPerSecond());
  app.start();
}
