
#include <cmath>

#include "gtest/gtest.h"

#include "al/io/al_AudioIO.hpp"
#include "al/math/al_Constants.hpp"
#include "al/system/al_Time.hpp"

using namespace al;

#ifndef TRAVIS_BUILD

TEST(Audio, AudioDeviceEnum) { AudioDevice::printAll(); }

static void callback(AudioIOData &io) {
  static double phase = 0.0;
  static double phaseInc = 2.0 * M_PI * 440.0 / io.framesPerSecond();
  while (io()) {
    io.out(0) = 0.1 * std::sin(phase);
    phase += phaseInc;
    if (phase > 2.0 * M_PI) {
      phase -= 2.0 * M_PI;
    }
  }
}

TEST(Audio, AudioIOObject) {
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
  EXPECT_TRUE(audioIO.user<int>() == 5);
  EXPECT_TRUE(audioIO.open());
  EXPECT_TRUE(audioIO.start());
  al_sleep(0.5);
  EXPECT_TRUE(audioIO.stop());
  EXPECT_TRUE(audioIO.close());
  EXPECT_TRUE(audioIO.user<int>() == 5);
}

TEST(Audio, VirtualChannels) {
  AudioIO audioIO;
  audioIO.init(nullptr, nullptr, 256, 44100, 1, 1);

  // Make sure parameters match those passed to constructor
  audioIO.open();
  EXPECT_TRUE(audioIO.framesPerBuffer() == 256);
  EXPECT_TRUE(audioIO.fps() == 44100);
  EXPECT_TRUE(audioIO.channelsOut() == 1);
  EXPECT_TRUE(audioIO.channelsIn() == 1);
  audioIO.close();

  // Test virtual channels
  audioIO.deviceIn(AudioDevice::defaultInput());
  audioIO.deviceOut(AudioDevice::defaultOutput());
  int maxChansOut = AudioDevice::defaultOutput().channelsOutMax();
  int maxChansIn = AudioDevice::defaultInput().channelsInMax();
  audioIO.channelsOut(maxChansOut + 1);
  audioIO.channelsIn(maxChansIn + 1);
  audioIO.open();
  EXPECT_TRUE(audioIO.channelsOutDevice() ==
              maxChansOut); // opened all hardware channels?
  EXPECT_TRUE(audioIO.channelsOut() ==
              (maxChansOut + 1)); // got our extra virtual channel?
  EXPECT_TRUE(audioIO.channelsInDevice() ==
              maxChansIn); // opened all hardware channels?
  EXPECT_TRUE(audioIO.channelsIn() ==
              (maxChansIn + 1)); // got our extra virtual channel?
  audioIO.close();
}

#else

#endif // TRAVIS_BUILD
