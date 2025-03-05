#include "al/sound/al_SoundFile.hpp"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#define DR_FLAC_IMPLEMENTATION
#include <cstdint>
#include <cstring>
#include <iostream>

#include "dr_flac.h"

using namespace al;

bool SoundFile::open(const char* path) {
  auto len = std::strlen(path);

  if (len < 5) {
    std::cerr << "not a valid file name: " << path << std::endl;
    return false;
  }

  const char* ext3 = path + (len - 4);
  if (std::strcmp(ext3, ".wav") == 0) {
    unsigned int c, s;
    uint64_t f;
    float* file_data =
        drwav_open_file_and_read_pcm_frames_f32(path, &c, &s, &f);
    if (file_data) {
      channels = (int)c;
      sampleRate = (int)s;
      frameCount = (long long int)f;
      size_t n = size_t(c * f);
      data.resize(n);
      std::memcpy(data.data(), file_data, sizeof(float) * n);
      drwav_free(file_data);
    } else {
      std::cerr << "failed to open file: " << path << std::endl;
      return false;
    }
    return true;
  } else if (std::strcmp(ext3, ".mp3") == 0) {
    std::cerr << "mp3 currently not supported\n";
    return false;
  }

  if (len < 6) {
    std::cerr << "not a valid file name: " << path << std::endl;
    return false;
  }

  const char* ext4 = path + (len - 5);
  if (std::strcmp(ext4, ".flac") == 0) {
    unsigned int c, s;
    drflac_uint64 f;
    float* file_data =
        drflac_open_file_and_read_pcm_frames_f32(path, &c, &s, &f);
    if (file_data) {
      channels = (int)c;
      sampleRate = (int)s;
      frameCount = (long long int)f;
      size_t n = (size_t)(c * f);
      data.resize(n);
      std::memcpy(data.data(), file_data, sizeof(float) * n);
      drflac_free(file_data);
    } else {
      std::cerr << "failed to open file: " << path << std::endl;
      return false;
    }
    return true;
  }
  return false;
}

float* SoundFile::getFrame(long long int frame) {
  return data.data() + frame * channels;
}

SoundFile al::getResampledSoundFile(SoundFile* toConvert,
                                    unsigned int newSampleRate) {
  // TODO!
  return {};
}

void SoundFilePlayer::getFrames(uint64_t numFrames, float* buffer,
                                int bufferLength) {
  if (pause || !soundFile) {
    for (int i = 0; i < bufferLength; i += 1) {
      buffer[i] = 0.0f;
    }
    return;
  }

  if (frame >= soundFile->frameCount) {
    if (loop) {
      frame = 0;
    } else {
      pause = true;
    }
    for (int i = 0; i < bufferLength; i += 1) {
      buffer[i] = 0.0f;
    }
    return;
  }

  int n = numFrames;
  int c = soundFile->channels;
  if (soundFile->frameCount < frame + n) {
    n = (int)(soundFile->frameCount - frame);
  }
  if (n * c >= bufferLength) {
    std::memcpy(buffer, soundFile->getFrame(frame),
                sizeof(float) * bufferLength);
  } else {
    std::memcpy(buffer, soundFile->getFrame(frame), sizeof(float) * n * c);
    for (int i = n * c; i < bufferLength; i += 1) {
      buffer[i] = 0.0f;
    }
  }
  frame += n;
}

SoundFileStreaming::SoundFileStreaming(const char* path) {
  if (path) {
    if (!open(path)) {
      std::cerr << "ERROR opening file:" << path << std::endl;
    }
  }
}

SoundFileStreaming::~SoundFileStreaming() { close(); }

uint32_t SoundFileStreaming::sampleRate() {
  return static_cast<drwav*>(mImpl)->sampleRate;
}

uint64_t SoundFileStreaming::totalFrames() {
  return static_cast<drwav*>(mImpl)->totalPCMFrameCount;
}

uint16_t SoundFileStreaming::numChannels() {
  return static_cast<drwav*>(mImpl)->channels;
}

bool SoundFileStreaming::open(const char* path) {
  close();
  mImpl = new drwav;
  if (!drwav_init_file((drwav*)mImpl, path)) {
    return false;
  }
  return true;
}

void SoundFileStreaming::close() {
  if (mImpl) {
    drwav_uninit((drwav*)mImpl);
    delete (drwav*)mImpl;
  }
}

uint64_t SoundFileStreaming::getFrames(uint64_t numFrames, float* buffer) {
  drwav_uint64 framesRead =
      drwav_read_pcm_frames_f32((drwav*)mImpl, numFrames, buffer);
  return framesRead;
}
