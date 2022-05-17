#include <iostream>
#include <memory>

#include "al/app/al_AudioDomain.hpp"
#include "al/app/al_ConsoleDomain.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/system/al_Time.hpp"

#include "Gamma/Oscillator.h"

// This example shows how to build an app class using only an
// AudioDomain to do audio I/O and a ConsoleDomain to
// keep the application running

using namespace al;

class MyAudioApp {
public:
  // The app will run an "AudioDomain"
  AudioDomain audioDomain;
  ConsoleDomain consoleDomain;

  gam::Sine<> mOsc{440};

  // Configure function
  void configure(double sampleRate, int blockSize, int audioOutputs,
                 int audioInputs) {
    // This configures the audio domain.
    audioDomain.configure(sampleRate, blockSize, audioOutputs, audioInputs);
  }

  // This start function starts the audio domain, waits for 3 seconds and
  // then exits
  void start() {
    audioDomain.init();
    consoleDomain.init();
    // Set audio callback through a lambda
    audioDomain.onSound = [this](AudioIOData &io) {
      while (io()) {
        io.out(0) = mOsc() * 0.1f;
      }
    };

    consoleDomain.onLine = [this](std::string line) {
      if (line.size() == 0) {
        return false;
      } else {
        std::cout << "Frequency:" << line << std::endl;
        mOsc.freq(std::stof(line));
      }
      return true;
    };
    // Set sample rate of Gamma from rate configured in audio domain
    // al::App will do this for you automatically, but you have to do it
    // manuall
    gam::sampleRate(audioDomain.audioIO().framesPerSecond());
    audioDomain.audioIO().print();
    // start audio domain. This domain is non blocking, so we will keep the
    // application alive by starting the console domain
    audioDomain.start();

    std::cout << "Enter a number to set the frequency. Press enter to quit."
              << std::endl;
    consoleDomain.start(); // Console Domain is a blocking domain

    // stop and cleanup domains
    audioDomain.stop();
    consoleDomain.stop();
    audioDomain.cleanup();
    consoleDomain.cleanup();
  }
};

int main() {
  MyAudioApp app;
  app.configure(44100, 256, 2, 0);
  app.start();

  return 0;
}
