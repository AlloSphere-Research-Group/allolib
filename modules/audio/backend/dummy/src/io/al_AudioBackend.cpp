#include "al/io/al_AudioIO.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring> /* memset() */
#include <cstring>
#include <iostream>
#include <string>

namespace al {

static void warn(const char *msg, const char *src) {
  fprintf(stderr, "%s%swarning: %s\n", src, src[0] ? " " : "", msg);
}

struct AudioBackendData {
  int numOutChans, numInChans;
  std::string streamName;
};

AudioBackend::AudioBackend() {
  mBackendData = std::make_shared<AudioBackendData>();
  static_cast<AudioBackendData *>(mBackendData.get())->numOutChans = 2;
  static_cast<AudioBackendData *>(mBackendData.get())->numInChans = 2;
  mOpen = false;
  mRunning = false;
}

bool AudioBackend::isOpen() const { return mOpen; }

bool AudioBackend::isRunning() const { return mRunning; }

bool AudioBackend::error() const { return false; }

void AudioBackend::printError(const char *text) const {
  if (error()) {
    fprintf(stderr, "%s: Dummy error.\n", text);
  }
}

void AudioBackend::printInfo() const {
  printf("Using dummy backend (no audio).\n");
}

bool AudioBackend::supportsFPS(double fps) { return true; }

void AudioBackend::inDevice(int index) { return; }

void AudioBackend::outDevice(int index) { return; }

void AudioBackend::channels(int num, bool forOutput) {
  if (forOutput) {
    setOutDeviceChans(num);
  } else {
    setInDeviceChans(num);
  }
}

int AudioBackend::inDeviceChans() { return 2; }

int AudioBackend::outDeviceChans() { return 2; }

void AudioBackend::setInDeviceChans(int num) {
  static_cast<AudioBackendData *>(mBackendData.get())->numInChans = num;
}

void AudioBackend::setOutDeviceChans(int num) {
  static_cast<AudioBackendData *>(mBackendData.get())->numOutChans = num;
}

double AudioBackend::time() { return 0.0; }

bool AudioBackend::open(int framesPerSecond, unsigned int framesPerBuffer,
                        void *userdata) {
  mOpen = true;
  return true;
}

bool AudioBackend::close() {
  mOpen = false;
  return true;
}

bool AudioBackend::start(int framesPerSecond, int framesPerBuffer,
                         void *userdata) {
  mRunning = true;
  return true;
}

bool AudioBackend::stop() {
  mRunning = false;
  return true;
}

double AudioBackend::cpu() { return 0.0; }

AudioDevice AudioBackend::defaultInput() { return AudioDevice(0); }

AudioDevice AudioBackend::defaultOutput() { return AudioDevice(0); }

int AudioBackend::numDevices() { return 1; }

int AudioBackend::deviceMaxInputChannels(int num) { return 2; }

int AudioBackend::deviceMaxOutputChannels(int num) { return 2; }

double AudioBackend::devicePreferredSamplingRate(int num) { return 44100; }

void AudioBackend::setStreamName(std::string name) {
  AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
  data->streamName = name;
}

std::string AudioBackend::deviceName(int num) { return "dummy_device"; }


} // namespace al
