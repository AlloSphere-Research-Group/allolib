#include "al/core/app/al_AudioApp.hpp"

using namespace al;

static void AppAudioCB(AudioIOData& io){
  AudioApp& app = io.user<AudioApp>();
  io.frame(0);
  app.onSound(app.audioIO());
}

void AudioApp::initAudio(
  double audioRate, int audioBlockSize,
  int audioOutputs, int audioInputs
) {
  mAudioIO.callback = AppAudioCB;
  mAudioIO.user(this);
  mAudioIO.framesPerSecond(audioRate);
  mAudioIO.framesPerBuffer(audioBlockSize);
  mAudioIO.channelsOut(audioOutputs);
  mAudioIO.channelsIn(audioInputs);
}

bool AudioApp::usingAudio() const {
  return audioIO().callback == AppAudioCB;
}

void AudioApp::begin() {
	if(usingAudio()) mAudioIO.start();
}

void AudioApp::end(){
	audioIO().close();
}