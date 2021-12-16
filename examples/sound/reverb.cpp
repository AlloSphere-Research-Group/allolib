/*
Allocore Example: Reverberation

Description:
This demonstrates how to apply a reverberation effect to the audio line input.

Author:
Lance Putnam, 4/25/2011, putnam.lance@gmail.com
*/

#include "al/app/al_App.hpp"
#include "al/sound/al_Reverb.hpp"
using namespace al;

struct MyApp : App {
  Reverb<float> reverb;

  void onInit() override {
    reverb.bandwidth(0.6f); // Low-pass amount on input, in [0,1]
    reverb.damping(0.5f);   // High-frequency damping, in [0,1]
    reverb.decay(0.6f);     // Tail decay factor, in [0,1]

    // Diffusion amounts
    // Values near 0.7 are recommended. Moving further away from 0.7 will lead
    // to more distinct echoes.
    reverb.diffusion(0.76, 0.666, 0.707, 0.571);
  }

  void onSound(AudioIOData &io) override {
    while (io()) {
      float dry = io.in(0);

      // Compute two wet channels of reverberation
      float wet1, wet2;
      reverb(dry, wet1, wet2);

      // Output just the wet signals
      io.out(0) = wet1 * 0.1;
      io.out(1) = wet2 * 0.1;
    }
  }
};

int main() {
  MyApp app;
  AudioDevice dev = AudioDevice::defaultOutput();
  app.configureAudio(dev, 44100, 256, 2, 2);
  app.start();
  return 0;
}
