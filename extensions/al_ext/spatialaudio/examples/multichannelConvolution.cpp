/*
*/

#include <iostream>

#include "al/core.hpp"

#include "al_ext/spatialaudio/al_Convolver.hpp"

#include "Gamma/Noise.h"
#include "Gamma/Envelope.h"

#define BLOCK_SIZE 64

using namespace std;
using namespace al;

struct MyApp : public App {
public:
  double phase;
  Convolver conv;
  vector<float *> IRchannels;

  void onInit() {
    // Make IR
    int irdur = 1.0; // duration of the IR in seconds
    int numFrames = audioIO().framesPerSecond() * irdur;

    // Gamma generators for decaying noise
    gam::sampleRate(audioIO().framesPerSecond());
    gam::NoisePink<> noise;
    gam::Decay<> env(numFrames);

    // Make two IRs from pink noise and a exponential decay.
    // You could also load the data from a soundfile
    for (int chan = 0; chan < 2; chan++) {
      float *ir = new  float[numFrames];
      for (int frame = 0; frame < numFrames; frame++) {
        ir[frame] = noise() * env() * 0.1;
      }
      env.reset();
      IRchannels.push_back(ir);
    }

    // Setup convolver. Map input 0 to 0, 1
    conv.configure(audioIO().framesPerBuffer(), IRchannels, numFrames, {{0, {0, 1}}});
  }

  //	// Audio callback
  void onSound(AudioIOData& io) override {
      // Copy input from sound card to convolution input
      memcpy(conv.getInputBuffer(0), io.inBuffer(0), io.framesPerBuffer() * sizeof (float));
      conv.processBuffer();
      memcpy(io.outBuffer(0), io.inBuffer(0), io.framesPerBuffer() * sizeof (float));
//      memcpy(io.outBuffer(1), conv.getOutputBuffer(1), io.framesPerBuffer() * sizeof (float));
  }

  void onExit() override {

    // Cleanup memory allocation
    for (unsigned int i = 0; i < IRchannels.size(); i++) {
      delete IRchannels[i];
    }
  }

};


int main(){
  double sampleRate = 44100;

  MyApp app;
  // Use this line to check devices on your system:
//  AudioDevice::printAll();
  app.initAudio(sampleRate, 1024, 2, 2);
  app.audioIO().print();
  app.start();
}
