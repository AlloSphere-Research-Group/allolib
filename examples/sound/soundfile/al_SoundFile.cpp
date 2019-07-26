#include "al_SoundFile.hpp"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

#include <cstring>
#include <iostream>

void al::SoundFile::open (const char* path) {
  auto len = std::strlen(path);

  if (len < 5) {
    std::cerr << "not a valid file name: " << path << std::endl;
    return;
  }

  const char* ext3 = path + (len-4);
  if (std::strcmp(ext3, ".wav") == 0) {
    unsigned int c, s;
    unsigned long long int f;
    float* file_data = drwav_open_file_and_read_pcm_frames_f32(path, &c, &s, &f);
    if (file_data) {
      channels = (int)c;
      sampleRate = (int)s;
      frameCount = (long long int)f;
      size_t n = size_t(channels * frameCount);
      data.resize(n);
      std::memcpy(data.data(), file_data, sizeof(float) * n);
      drwav_free(file_data);
    }
    else {
      std::cerr << "failed to open file: " << path << std::endl;
    }
    return;
  }
  else if (std::strcmp(ext3, ".mp3") == 0) {
    std::cerr << "mp3 currently not supported\n";
  }

  if (len < 6) {
    std::cerr << "not a valid file name: " << path << std::endl;
    return;
  }

  const char* ext4 = path + (len-5);
  if (std::strcmp(ext4, ".flac") == 0) {
    unsigned int c, s;
    unsigned long long int f;
    float* file_data = drflac_open_file_and_read_pcm_frames_f32(path, &c, &s, &f);
    if (file_data) {
      channels = (int)c;
      sampleRate = (int)s;
      frameCount = (long long int)f;
      size_t n = (size_t)(channels * frameCount);
      data.resize(n);
      std::memcpy(data.data(), file_data, sizeof(float) * n);
      drflac_free(file_data);
    }
    else {
      std::cerr << "failed to open file: " << path << std::endl;
    }
    return;
  }
}

float* al::SoundFile::getFrame (long long int frame) {
  return data.data() + frame * channels;
}

al::SoundFile al::getResampledSoundFile (SoundFile* toConvert, unsigned int newSampleRate) {
  // TODO!
  return {};
}
 
void al::SoundFilePlayer::getFrames (int numFrames, float* buffer, int bufferLength) {
  if (pause || !soundFile) {
    for (int i = 0; i < bufferLength; i += 1) {
      buffer[i] = 0.0f;
    }
    return;
  }

  if (frame >= soundFile->frameCount) {
    if (loop) {
      frame = 0;
    }
    else {
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
    std::memcpy(buffer, soundFile->getFrame(frame), sizeof(float) * bufferLength);
  }
  else {
    std::memcpy(buffer, soundFile->getFrame(frame), sizeof(float) * n * c);
    for (int i = n * c; i < bufferLength; i += 1)
    {
        buffer[i] = 0.0f;
    }
  }
  frame += n;
}
