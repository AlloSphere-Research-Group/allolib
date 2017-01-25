#pragma once

#include "al/core/app/al_AudioIO.hpp"

namespace al {

class AudioApp {
public:
  AudioIO mAudioIO;
  AudioIO& audioIO(){ return mAudioIO; }
  const AudioIO& audioIO() const { return mAudioIO; }

  /// Initialize audio
  /// @param[in] sampleRate. Unsupported values will use default rate of device.
  /// @param[in] blockSize. Number of sample frames to process per callback
  /// @param[in] outputChannels. Number of output channels to open. -1 for all
  /// @param[in] inputChannels. Number of input channels to open. -1 for all
  void initAudio(
    double audioRate=44100, int audioBlockSize=128,
    int audioOutputs=-1, int audioInputs=-1
  );
  
  bool usingAudio() const;
  void begin();
  void end();

  virtual void onSound(AudioIOData& io) {}
    
};

}

