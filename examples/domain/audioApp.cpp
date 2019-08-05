#include <memory>
#include <iostream>

#include "al/app/al_AudioDomain.hpp"
#include "al/graphics/al_Shapes.hpp"

#include "Gamma/Oscillator.h"

using namespace al;

class MyAudioApp {
public:

  // The app will run an "AudioDomain"
  AudioDomain audioDomain;

  gam::Sine<> mOsc {440};

  // Configure function
  void configure(double sampleRate, int blockSize, int audioOutputs, int audioInputs) {
    // This configures the audio domain.
    audioDomain.configure(sampleRate, blockSize, audioOutputs, audioInputs);
  }

  // This start function starts the audio domain, waits for 3 seconds and
  // then exits
  void start() {
    audioDomain.initialize();
    // Set audio callback through a lambda
    audioDomain.onSound = [this](AudioIOData &io) {
      while(io()) {
        io.out(0) =  mOsc() * 0.1f;
      }
    };
    // Set sample rate of Gamma from rate configured in audio domain
    gam::sampleRate(audioDomain.audioIO().framesPerSecond());
    audioDomain.audioIO().print();
    // start audio domain. This domain is non blocking, so we will keep the
    // application alive by calling al_sleep()
    audioDomain.start();

    al_sleep(3.0);

    // stop and cleanup domains
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
