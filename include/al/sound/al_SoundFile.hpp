#ifndef INCLUDE_AL_SOUNDFILE_HPP
#define INCLUDE_AL_SOUNDFILE_HPP

#include <atomic>
#include <vector>

namespace al {

/// @brief Read sound file and store the data in float array (interleaved)
/// @ingroup Sound
/// Reading supports wav, flac
/// Implementation uses "dr libs" (https://github.com/mackron/dr_libs)

struct SoundFile {
  std::vector<float> data;
  int sampleRate = 0;
  int channels = 0;
  long long int frameCount = 0;

  // In case of adding some constructor other than default constructor,
  //   remember to implement or explicitly specify related functions
  // Related concept: `Rule of 5`

  //  SoundFile() = default;
  //  SoundFile(const SoundFile&) = default;
  //  SoundFile(SoundFile&&) noexcept = default;
  //
  //  SoundFile& operator=(const SoundFile&) = default;
  //  SoundFile& operator=(SoundFile&&) noexcept = default;
  //
  //  ~SoundFile() = default;

  bool open(const char* path);
  float* getFrame(long long int frame);  // unsafe, without frameCount check
};

SoundFile getResampledSoundFile(SoundFile* toConvert,
                                unsigned int newSampleRate);

// only supports wav
// TODO - implement
// void writeSoundFile (const char* filename, float* data, int frameCount,
//                      int sampleRate, int channels);

/// @brief Soundfile player class
/// @ingroup Sound
struct SoundFilePlayer {
  long long int frame = 0;
  bool pause = true;
  bool loop = false;
  SoundFile* soundFile = nullptr;  // non-owning

  // In case of adding some constructor other than default constructor,
  //   remember to implement or explicitly specify related functions
  // Related concept: `Rule of 5`

  //  SoundFilePlayer() = default;
  //  SoundFilePlayer(const SoundFilePlayer&) = default;
  //  SoundFilePlayer(SoundFilePlayer&&) noexcept = default;
  //
  //  SoundFilePlayer& operator=(const SoundFilePlayer&) = default;
  //  SoundFilePlayer& operator=(SoundFilePlayer&&) noexcept = default;
  //
  //  ~SoundFilePlayer() = default;

  void getFrames(int numFrames, float* buffer, int bufferLength);
};

/// @brief Soundfile player class
/// @ingroup Sound
struct SoundFilePlayerTS {
  SoundFile soundFile;
  SoundFilePlayer player;
  std::atomic<bool> pauseSignal;
  std::atomic<bool> loopSignal;
  std::atomic<bool> rewindSignal;
  // TODO - volume and fading

  // In case of adding some constructor other than default constructor,
  //   remember to implement or explicitly specify related functions
  // Related concept: `Rule of 5`

  //  SoundFilePlayerTS() = default;
  //  SoundFilePlayerTS(const SoundFilePlayerTS&) = default;
  //  SoundFilePlayerTS(SoundFilePlayerTS&&) noexcept = default;
  //
  //  SoundFilePlayerTS& operator=(const SoundFilePlayerTS&) = default;
  //  SoundFilePlayerTS& operator=(SoundFilePlayerTS&&) noexcept = default;
  //
  //  ~SoundFilePlayerTS() = default;

  bool open(const char* path) {
    bool ret = soundFile.open(path);
    player.soundFile = &soundFile;
    return ret;
  }

  void setPlay() { pauseSignal.store(false); }
  void setPause() { pauseSignal.store(true); }
  void togglePause() { pauseSignal.store(!pauseSignal.load()); }

  void setRewind() { rewindSignal.store(true); }

  void setLoop() { loopSignal.store(true); }
  void setNoLoop() { loopSignal.store(false); }

  void getFrames(int numFrames, float* buffer, int bufferLength) {
    player.pause = pauseSignal.load();
    player.loop = loopSignal.load();
    if (rewindSignal.exchange(false)) {
      player.frame = 0;
    }
    player.getFrames(numFrames, buffer, bufferLength);
  }
};

}  // namespace al

#endif
