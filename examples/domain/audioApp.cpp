#include <memory>
#include <iostream>

#include "al/domains/al_OSCDomain.hpp"
#include "al/domains/al_AudioDomain.hpp"
#include "al/graphics/al_Shapes.hpp"

#include "Gamma/Oscillator.h"

using namespace al;

class MyAudioApp {
public:

  // Domains to build the app with
  AudioDomain audioDomain;
  OSCDomain oscDomain;

  gam::Sine<> mOsc {440};

  void configure(double sampleRate, int blockSize, int audioOutputs, int audioInputs) {
    audioDomain.configure(sampleRate, blockSize, audioOutputs, audioInputs);
  }

  void start() {
    audioDomain.initialize();
    // Audio callback
    audioDomain.onSound = [this](AudioIOData &io) {
      while(io()) {
        io.out(0) =  mOsc() * 0.1f;
      }
    };
    gam::sampleRate(audioDomain.audioIO().framesPerSecond());
    audioDomain.audioIO().print();
    audioDomain.start();

    al_sleep(3.0);

    audioDomain.stop();
    audioDomain.cleanup();
  }

};


int main()
{
  MyAudioApp app;
  app.configure(44100, 256, 2, 0);
  app.start();

  return 0;
}
