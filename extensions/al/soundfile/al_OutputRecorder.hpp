#ifndef OUTPUTRECORDER_H
#define OUTPUTRECORDER_H

#include "al/core/io/al_AudioIOData.hpp"
#include "al/soundfile/al_SoundfileBufferedRecord.hpp"

namespace al {

class OutputRecorder : public AudioCallback, public SoundFileBufferedRecord {
public:
  bool start(std::string fullPath, double frameRate, uint32_t numChannels,
             uint32_t bufferFrames = 4096,
             Format format = Format::WAV, EncodingType encoding = EncodingType::PCM_16) {
    mBuffers.clear();
    return SoundFileBufferedRecord::open(fullPath, frameRate, numChannels,
                                         bufferFrames, format, encoding);
  }

  void stop() {
    SoundFileBufferedRecord::close();
  }

  void onAudioCB(AudioIOData &io) {
    if (SoundFileBufferedRecord::opened()) {
      if (mBuffers.size() == 0) {
        // First run. Put output buffers in buffers array
        // This needs to be done only once and assumes the buffers will always
        // be in the same location
        for(int i = 0; i < std::min(int(io.channelsOut()), mSf.channels()); i++) {
          mBuffers.push_back(io.outBuffer(i));
        }
      }
      SoundFileBufferedRecord::write(mBuffers, io.framesPerBuffer());
    }
  }

private:
  std::vector<float *> mBuffers;
};

}
#endif //OUTPUTRECORDER_H
