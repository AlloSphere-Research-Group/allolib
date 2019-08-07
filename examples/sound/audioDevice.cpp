/*
Allocore Example: Audio System Probe

Description:
A useful tool to test channels etc. of an audio device

Author:
Graham Wakefield, 2011
*/

#include <cstdio>
#include <vector>
#include "al/app/al_App.hpp"
#include "al/math/al_StdRandom.hpp"

using namespace al;

// This example shows how to select Audio devices for an app in different ways

struct MyApp : App {
  void onSound(AudioIOData& io) override {
    while (io()) {
      // Play noise on the first output channel
      io.out(0) = rnd::uniformS() * 0.1f;
    }
  }
};

int main() {
  // Default devices
  AudioDevice dev = AudioDevice::defaultOutput();
  dev.print();

  AudioDevice dev_in = AudioDevice::defaultInput();
  dev_in.print();

  // You can also specify a device by name:
  AudioDevice dev_name = AudioDevice("Device Name");
  // This is probably an invalid device. This will be printed.
  dev_name.print();

  // You can get a list of available devices with their names
  for (int i = 0; i < AudioDevice::numDevices(); i++) {
    printf(" --- [%2d] ", i);
    AudioDevice dev(i);
    dev.print();
  }
  // This is equivalent to:
  //  AudioDevice::printAll();

  MyApp app;

  // You must initialize audio device before starting application to enable
  // audio You will often want to set all values from the device itself (except
  // block size):
  app.configureAudio(dev, dev.defaultSampleRate(), 256, dev.channelsOutMax(),
                     dev.channelsInMax());
  // Or you can use defaults for everything
  //  app.configureAudio();

  // If you need more options, you can request the audioIO object from the audio
  // domain in the app
  app.audioDomain()->audioIO().print();

  app.start();

  return 0;
}
