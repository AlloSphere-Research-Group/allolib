#include "al/core/app/al_AudioApp.hpp"

using namespace al;

static void AppAudioCB(AudioIOData& io){
  AudioApp& app = io.user<AudioApp>();
  io.frame(0);
  app.onSound(app.audioIO());
}

void AudioApp::initAudio(
  double audioRate, int audioBlockSize,
  int audioOutputs, int audioInputs,
  int device
) {
  mAudioIO.init(AppAudioCB, this, audioBlockSize, audioRate, audioOutputs, audioInputs);
  if (device >= 0) {
       mAudioIO.device(AudioDevice(device));
       // mAudioIO.device() sets the channels to the device default number
       mAudioIO.channelsIn(audioInputs);
       mAudioIO.channelsOut(audioOutputs);
  }
  mAudioIO.open();
}

void AudioApp::initAudio(AudioIOConfig config) {
    bool use_in = (config & IN_ONLY) ? true : false;
    bool use_out = (config & OUT_ONLY) ? true : false;
    mAudioIO.initWithDefaults(AppAudioCB, this, use_out, use_in);
    mAudioIO.open();
}

bool AudioApp::usingAudio() const {
  return audioIO().callback == AppAudioCB;
}

void AudioApp::beginAudio() {
  if(usingAudio()) {
      mAudioIO.start();
  }
}

void AudioApp::endAudio(){
  audioIO().stop();
	audioIO().close();
}
