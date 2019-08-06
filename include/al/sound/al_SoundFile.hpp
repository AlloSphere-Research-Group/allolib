#ifndef INCLUDE_AL_SOUNDFILE_HPP
#define INCLUDE_AL_SOUNDFILE_HPP

#include <vector>
#include <atomic>

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

struct SoundFilePlayerTS
{
  SoundFile soundFile;
  SoundFilePlayer player;
  std::atomic<bool> pauseSignal;
  std::atomic<bool> loopSignal;
  std::atomic<bool> rewindSignal;
  // TODO - volume and fading

  SoundFilePlayerTS () = default;
  SoundFilePlayerTS (const SoundFilePlayerTS& other) = default;
  SoundFilePlayerTS (SoundFilePlayerTS&& other) = default;

  SoundFilePlayerTS& operator= (const SoundFilePlayerTS& other) = default;
  SoundFilePlayerTS& operator= (SoundFilePlayerTS&& other) = default;

  ~SoundFilePlayerTS () = default;

  void open (const char* path) {
      soundFile.open(path);
      player.soundFile = &soundFile;
  }

  void setPlay () { pauseSignal.store(true); }
  void setPause () { pauseSignal.store(false); }
  void togglePause () { pauseSignal.store(pauseSignal.load()); }

  void rewind () { rewindSignal.store(true); }

  void setLoop () { loopSignal.store(true); }
  void setNoLoop () {loopSignal.store(false); }

  void getFrames (int numFrames, float* buffer, int bufferLength) {
    player.pause = pauseSignal.load();
    player.loop = loopSignal.load();
    if (rewindSignal.exchange(false)) {
        player.frame = 0;
    }
    player.getFrames(numFrames, buffer, bufferLength);
  }
};

} // namespace al

#endif

