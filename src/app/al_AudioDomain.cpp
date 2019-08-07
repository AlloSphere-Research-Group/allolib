#include "al/app/al_AudioDomain.hpp"

using namespace al;


bool AudioDomain::initialize(ComputationDomain *parent) {
  bool ret = true;

  callInitializeCallbacks();
  ret &= audioIO().open();
  gam::Domain::spu(audioIO().framesPerSecond());
  return ret;
}

bool AudioDomain::start() {
  bool ret = true;
  ret &= audioIO().start();
  return ret;
}

bool AudioDomain::stop() {
  bool ret = true;
  ret &= audioIO().stop();
  ret &= audioIO().close();
  return true;
}

bool AudioDomain::cleanup(ComputationDomain */*parent*/) {
  callCleanupCallbacks();
  return true;
}

void AudioDomain::configure(double audioRate, int audioBlockSize, int audioOutputs, int audioInputs) {
  AudioDevice dev = AudioDevice::defaultOutput();
  configure(dev, audioRate, audioBlockSize,
            audioOutputs, audioInputs);
}

void AudioDomain::configure(AudioDevice &dev, double audioRate, int audioBlockSize, int audioOutputs, int audioInputs)
{
  audioIO().init(AudioDomain::AppAudioCB, this, audioBlockSize, audioRate, audioOutputs, audioInputs);
  audioIO().device(dev);
  // mAudioIO.device() resets the channels to the device default number
  audioIO().channelsIn(audioInputs);
  audioIO().channelsOut(audioOutputs);
}
