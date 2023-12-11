#include "al/app/al_AudioDomain.hpp"

using namespace al;

AudioDomain::AudioDomain() {
  mParameters = {&mGainParameter};

  mGainParameter.registerChangeCallback(
      [&](float value) { mAudioIO.gain(value); });
}

bool AudioDomain::init(ComputationDomain *parent) {
  (void)parent;
  bool ret = true;
  callInitializeCallbacks();
  return ret;
}

bool AudioDomain::start() {
  bool ret = true;
  if (audioIO().channelsIn() > 0 && audioIO().channelsOut() > 0) {
    ret &= audioIO().open();
    gam::sampleRate(audioIO().framesPerSecond());
    ret &= audioIO().start();
  }
  return ret;
}

bool AudioDomain::stop() {
  bool ret = true;
  ret &= audioIO().stop();
  ret &= audioIO().close();
  return ret;
}

bool AudioDomain::cleanup(ComputationDomain * /*parent*/) {
  callCleanupCallbacks();
  return true;
}

void AudioDomain::configure(double audioRate, int audioBlockSize,
                            int audioOutputs, int audioInputs) {
  AudioDevice dev = AudioDevice::defaultOutput();
  configure(dev, audioRate, audioBlockSize, audioOutputs, audioInputs);
}

void AudioDomain::configure(AudioDevice &dev, double audioRate,
                            int audioBlockSize, int audioOutputs,
                            int audioInputs) {
  audioIO().init(AudioDomain::AppAudioCB, this, audioBlockSize, audioRate,
                 audioOutputs, audioInputs);
  audioIO().device(dev);
  // mAudioIO.device() resets the channels to the device default number
  audioIO().channelsIn(audioInputs);
  audioIO().channelsOut(audioOutputs);
}

void AudioDomain::AppAudioCB(AudioIOData &io) {
  AudioDomain &app = io.user<AudioDomain>();
  io.frame(0);
  app.onSound(app.audioIO());
}

// -----
bool GammaAudioDomain::start() {
  bool ret = true;
  if (audioIO().channelsIn() > 0 || audioIO().channelsOut() > 0) {
    ret &= audioIO().open();
    gam::Domain::spu(audioIO().framesPerSecond()); // Set for this object
    gam::sampleRate(audioIO().framesPerSecond());
    ret &= audioIO().start();
  }
  return ret;
}
