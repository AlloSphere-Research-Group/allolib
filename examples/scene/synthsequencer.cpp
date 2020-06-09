/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:
    Description:
*/

#include <cstdio>        // for printing to stdout
#define GAMMA_H_INC_ALL  // define this to include all header files
#define GAMMA_H_NO_IO    // define this to avoid bringing AudioIO from Gamma

#include "Gamma/Gamma.h"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/io/al_AudioIO.hpp"
#include "al/scene/al_SynthSequencer.hpp"

using namespace gam;
using namespace al;

// This example shows algorithmic generation of events for an audiovisual
// "SynthSequencer".

// Create a sub class of SynthVoice to determine what each voice should do
// in the onProcess() audio and video callbacks.
// Add functions to set voice parameters (per instance parameters)
// Don't forget to define an onTriggerOn() function to reset envelopes or
// values for each triggering
class SineEnv : public SynthVoice {
 public:
  float mAmp;
  float mDur;

  Sine<> mOsc;

  Mesh mMesh;

  void onInit() { addSphere(mMesh, 0.2, 30, 30); }

  void onProcess(AudioIOData &io) override {
    while (io()) {
      float s1 = mOsc() * 0.05;
      io.out(0) += s1;
      io.out(1) += s1;
    }
  }

  void onProcess(Graphics &g) override {
    g.pushMatrix();
    //        g.blending(true);
    //        g.blendTrans();
    g.translate(mOsc.freq() / 500 - 3, pow(mAmp, 0.3), -8);
    g.scale(1 - mDur, mDur, 1);
    g.color(1, mOsc.freq() / 1000, 1.0);
    g.draw(mMesh);
    g.popMatrix();
  }

  void onTriggerOn() override { std::cout << "hello" << std::endl; }
};

// make an app that contains a SynthSequencer class
// use the render() functions from the SynthSequencer to produce audio and
// graphics in the corresponding callback
class MyApp : public App {
 public:
  void onSound(AudioIOData &io) override {
    s.render(io);  // Render audio
  }

  void onDraw(Graphics &g) override {
    s.print();  // Prints information on active and free voices
    g.clear(0.4);
    s.render(g);  // Render graphics
  }

  SynthSequencer s;
};

// Function to quickly create a set of notes
void makeLine(SynthSequencer &s, float startTime, int number, float duration,
              float startFreq, float endFreq) {
  for (int i = 0; i < number; i++) {
    // Add notes to the sequencer. The add function returns a reference to a
    // voice so you can call functions you have defined to configure it.
    s.add<SineEnv>(startTime + i * duration, duration)
        .mOsc.freq(startFreq + ((endFreq - startFreq) * i / (float)number));
  }
}

int main() {
  // Create app instance
  MyApp app;

  // Sequence events using the function defined above
  makeLine(app.s, 0, 10, 0.6, 440, 660);
  makeLine(app.s, 3, 10, 0.4, 220, 660);
  makeLine(app.s, 6, 10, 0.3, 220, 440);
  makeLine(app.s, 10, 30, 0.2, 220, 440);
  makeLine(app.s, 13, 80, 0.1, 220, 880);
  makeLine(app.s, 18, 180, 0.05, 220, 440);
  makeLine(app.s, 20, 10, 0.1, 880, 1600);
  makeLine(app.s, 20, 10, 0.2, 880, 1600);
  makeLine(app.s, 21, 10, 0.3, 440, 1600);
  makeLine(app.s, 22, 20, 0.1, 220, 3000);
  makeLine(app.s, 25, 30, 0.4, 3000, 1500);
  makeLine(app.s, 27, 30, 0.5, 4000, 1000);
  makeLine(app.s, 30, 50, 0.6, 3000, 500);
  makeLine(app.s, 31, 10, 1.0, 880, 440);

  // Start everything
  app.configureAudio(44100., 256, 2, 0);
  Domain::master().spu(app.audioIO().framesPerSecond());
  app.start();
  return 0;
}
