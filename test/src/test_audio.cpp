
#include <cmath>

#include "al/io/al_AudioIO.hpp"
#include "al/math/al_Constants.hpp"
#include "al/system/al_Time.hpp"
#include "catch.hpp"

using namespace al;

#ifndef TRAVIS_BUILD

TEST_CASE("Audio Device Enum") { AudioDevice::printAll(); }

static void callback(AudioIOData &io) {
  static double phase = 0.0;
  static double phaseInc = 2.0 * M_PI * 440.0 / io.framesPerSecond();
  while (io()) {
    io.out(0) = std::sin(phase);
    phase += phaseInc;
    if (phase > 2.0 * M_PI) {
      phase -= 2.0 * M_PI;
    }
  }
}

TEST_CASE("Audio IO Object") {
  int userData = 5;
  AudioIO audioIO;
#ifdef AL_WINDOWS
  bool use_out = true;
  bool use_in = false;
  audioIO.initWithDefaults(callback, &userData, use_out, use_in);
#else
  audioIO.init(callback, &userData, 64, 44100.0, 2, 2);
#endif
  audioIO.print();
  REQUIRE(audioIO.user<int>() == 5);
  REQUIRE(audioIO.open());
  REQUIRE(audioIO.start());
  al_sleep(0.5);
  REQUIRE(audioIO.stop());
  REQUIRE(audioIO.close());
  REQUIRE(audioIO.user<int>() == 5);
}

TEST_CASE("Audio Channels/Virtual Channels") {
  AudioIO audioIO;
  audioIO.init(nullptr, nullptr, 256, 44100, 1, 1);

  // Make sure parameters match those passed to constructor
  audioIO.open();
  REQUIRE(audioIO.framesPerBuffer() == 256);
  REQUIRE(audioIO.fps() == 44100);
  REQUIRE(audioIO.channelsOut() == 1);
  REQUIRE(audioIO.channelsIn() == 1);
  audioIO.close();

  // Test virtual channels
  audioIO.deviceIn(AudioDevice::defaultInput());
  audioIO.deviceOut(AudioDevice::defaultOutput());
  int maxChansOut = AudioDevice::defaultOutput().channelsOutMax();
  int maxChansIn = AudioDevice::defaultInput().channelsInMax();
  audioIO.channelsOut(maxChansOut + 1);
  audioIO.channelsIn(maxChansIn + 1);
  audioIO.open();
  REQUIRE(audioIO.channelsOutDevice() ==
          maxChansOut);  // opened all hardware channels?
  REQUIRE(audioIO.channelsOut() ==
          (maxChansOut + 1));  // got our extra virtual channel?
  REQUIRE(audioIO.channelsInDevice() ==
          maxChansIn);  // opened all hardware channels?
  REQUIRE(audioIO.channelsIn() ==
          (maxChansIn + 1));  // got our extra virtual channel?
  audioIO.close();
}

#else

#endif  // TRAVIS_BUILD
