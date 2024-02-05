#ifndef INCLUDE_AL_SOUNDFILE_HPP
#define INCLUDE_AL_SOUNDFILE_HPP

#include <atomic>
#include <vector>

namespace al {

/**
 * @brief Read sound file and store the data in float array (interleaved)
 * @ingroup Sound
 *
 * Reading supports wav, flac
 * Implementation uses "dr libs" (https://github.com/mackron/dr_libs)
 */
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

  void getFrames(uint64_t numFrames, float* buffer, int bufferLength);
};

/**
 * @brief The SoundFileStreaming class provides reading soundifle directly from
 * disk one buffer at a time.
 *
 * This is a simple reading class with few options, if you need more
 * comprehensive support, use the soundfile module in al_ext
 */
class SoundFileStreaming {
 public:
  SoundFileStreaming(const char* path = nullptr);
  ~SoundFileStreaming();

  bool isOpen() { return mImpl != nullptr; }

  /// Sampling rate of file. Call after open has returned true.
  uint32_t sampleRate();
  /// Total number of frames in file. Call after open has returned true.
  uint64_t totalFrames();
  /// Number of channels in file. Call after open has returned true.
  uint16_t numChannels();

  /// Open file for reading.
  bool open(const char* path);
  /// Close file and cleanup
  void close();
  /// Read interleaved frames into preallocated buffer;
  uint64_t getFrames(uint64_t numFrames, float* buffer);

 private:
  void* mImpl{nullptr};
};

/// @brief Soundfile player class with thread-safe access to playback controls
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
