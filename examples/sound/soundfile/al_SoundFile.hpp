#ifndef INCLUDE_AL_SOUNDFILE_HPP
#define INCLUDE_AL_SOUNDFILE_HPP

#include <vector>

namespace al {

// read sound file and store the data in float array (interleaved)
// reading supports wav, flac
// implementation uses "dr libs" (https://github.com/mackron/dr_libs)

struct SoundFile
{
  std::vector<float> data;
  int sampleRate = 0;
  int channels = 0;
  long long int frameCount = 0;

  SoundFile () = default;
  SoundFile (const SoundFile& other) = default;
  SoundFile (SoundFile&& other) = default;

  SoundFile& operator= (const SoundFile& other) = default;
  SoundFile& operator= (SoundFile&& other) = default;

  ~SoundFile () = default;

  void open (const char* path);
  float* getFrame (long long int frame); // unsafe, without frameCount check
};

SoundFile getResampledSoundFile (SoundFile* toConvert, unsigned int newSampleRate);

struct SoundFilePlayer
{
  long long int frame = 0;
  bool pause = true;
  bool loop = false;
  SoundFile* soundFile = nullptr; // non-owning

  SoundFilePlayer () = default;
  SoundFilePlayer (const SoundFilePlayer& other) = default;
  SoundFilePlayer (SoundFilePlayer&& other) = default;

  SoundFilePlayer& operator= (const SoundFilePlayer& other) = default;
  SoundFilePlayer& operator= (SoundFilePlayer&& other) = default;

  ~SoundFilePlayer () = default;

  void getFrames (int numFrames, float* buffer, int bufferLength);
};

// only supports wav
//void writeSoundFile (const char* filename, float* data, int frameCount, int sampleRate, int channels);

} // namespace al

#endif

