#include "al_AudioDomain.hpp"

using namespace al;


bool AudioDomain::initialize(ComputationDomain *parent) {
  callInitializeCallbacks();
  return true;
}

bool AudioDomain::start() {
  bool ret = true;
  ret &= audioIO().open();
  gam::Domain::spu(audioIO().framesPerSecond());
  ret &= audioIO().start();
  return ret;
}

bool AudioDomain::stop() {
  bool ret = true;
  ret &= audioIO().stop();
  ret &= audioIO().close();
  return true;
}

bool AudioDomain::cleanup(ComputationDomain *parent) {
  callCleanupCallbacks();
  return true;
}

void AudioDomain::configure(double audioRate, int audioBlockSize, int audioOutputs, int audioInputs, int device) {
  AudioDevice dev = AudioDevice(device);
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

void AudioDomain::configure(AudioDomain::AudioIOConfig config) {
  bool use_in = (config & IN_ONLY) ? true : false;
  bool use_out = (config & OUT_ONLY) ? true : false;
  audioIO().initWithDefaults(AppAudioCB, this, use_out, use_in);
}
