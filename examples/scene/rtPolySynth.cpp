
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/io/al_AudioIO.hpp"
#include "al/scene/al_PolySynth.hpp"

using namespace gam;
using namespace al;

// Inherit from SynthVoice to determine what each voice should do
// in the onProcess() audio and video callbacks.
// Add functions to set voice parameters (per instance parameters)
// Don't forget to define an onTriggerOn() function to reset envelopes or
// values for each triggering and an onTriggerOff() function to
// determine what the note should do when it is deactivated
class SineEnv : public SynthVoice {
 public:
  SineEnv() {
    mAmpEnv.curve(0);  // make segments lines
    mAmpEnv.sustainPoint(2);

    addSphere(mMesh, 0.5, 30, 30);
  }

  // Note parameters
  SineEnv& freq(float v) {
    mOsc.freq(v);
    return *this;
  }

  // Audio processing function
  void onProcess(AudioIOData& io) override {
    while (io()) {
      float s = mOsc() * mAmpEnv() * mAmp;
      io.out(0) += s;
      io.out(1) += s;
    }
    if (mAmpEnv.done()) free();
  }

  // Graphics processing function
  void onProcess(Graphics& g) override {
    float spatialEnv = mAmpEnv.value();
    g.pushMatrix();
    g.blending(true);
    g.blendTrans();
    g.translate(mOsc.freq() / 250 - 3, spatialEnv * 2 - 1, -8);
    g.color(spatialEnv, mOsc.freq() / 1000, spatialEnv, spatialEnv);
    g.draw(mMesh);
    g.popMatrix();
  }

  void onTriggerOn() override { mAmpEnv.reset(); }

  void onTriggerOff() override { mAmpEnv.release(); }

 protected:
  float mAmp{0.2f};
  float mDur{1.5f};
  Sine<> mOsc;
  Env<3> mAmpEnv{0.f, 0.5f, 1.f, 1.0f, 1.f, 2.0f, 0.f};

  Mesh mMesh;
};

// We will use PolySynth to handle voice triggering and allocation
struct MyApp : public App {
  PolySynth pSynth;

  void onCreate() override {
    // Pre-allocate voice to avoid real-time allocation
    pSynth.allocatePolyphony<SineEnv>(16);

    navControl().active(false);  // Disable navigation via keyboard, since we
                                 // will be using keyboard for note triggering
  }

  void onSound(AudioIOData& io) override {
    pSynth.render(io);  // Render audio
  }

  void onDraw(Graphics& g) override {
    g.clear();
    pSynth.render(g);
  }

  bool onKeyDown(Keyboard const& k) override {
    int midiNote = asciiToMIDI(k.key());
    if (midiNote > 0) {
      float frequency = ::pow(2., (midiNote - 69.) / 12.) * 440.;
      SineEnv* voice = pSynth.getVoice<SineEnv>();
      voice->freq(frequency);
      pSynth.triggerOn(voice, 0, midiNote);
    }
    return true;
  }

  bool onKeyUp(Keyboard const& k) override {
    int midiNote = asciiToMIDI(k.key());
    if (midiNote > 0) {
      pSynth.triggerOff(midiNote);
    }
    return true;
  }
};

int main() {
  // Create app instance
  MyApp app;

  // Start audio
  app.configureAudio(44100., 256, 2, 0);

  // Set up sampling rate for Gamma objects
  Domain::master().spu(app.audioIO().framesPerSecond());

  app.start();
}
