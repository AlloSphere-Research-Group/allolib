#ifndef SOUNDFILEBUFFERED_H
#define SOUNDFILEBUFFERED_H


#include <mutex>
#include <thread>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <functional>
#include <algorithm>

#include "Gamma/SoundFile.h"
#include "al/core/types/al_SingleRWRingBuffer.hpp"
#include "al/core/io/al_AudioIOData.hpp"


namespace al
{

///
/// \brief Write a soundfile with buffering on a low priority thread
///
/// The SoundFileBuffered class is a wrapper around Gamma's SoundFile class.
/// It writes the soundfile in a separate thread and buffering is done with a
/// lock-free ring buffer from the audio callback
///
///
class SoundFileBufferedRecord
{
public:

  typedef gam::SoundFile::Format Format;
  typedef gam::SoundFile::EncodingType EncodingType;

  ///
  /// \param fullPath The full path to the audio file
  /// \param loop set to true if you want the sound file to start over when finished
  /// \param bufferFrames the size of the ring buffer. Set to larger if experiencing dropouts or if planning to read more samples, e.g. the audio buffer size is large.
  ///
  SoundFileBufferedRecord();
  ~SoundFileBufferedRecord();

  bool open(std::string fullPath, double frameRate, uint32_t numChannels,
            uint32_t bufferFrames = 8192,
            Format format = Format::WAV, EncodingType encoding = EncodingType::PCM_16);

  void cleanup();

  bool close();

  ///
  /// \brief Write audio file from separate audio buffers
  /// \param buffers vector of buffers to write
  /// \param numFrames number of frames in each buffer
  /// \return the number of frames actually read.
  ///
  void write(std::vector<float *>buffers, size_t numFrames);

  void setMaxWriteTime(float maxTime) {mMaxWriteTime = maxTime;}

  bool opened() const;								///< Returns whether the sound file is open

//  typedef std::function<void(float *buffer, int numChannels,
//                             int numFrames, void * userData)> CallbackFunc;

//  ///
//  /// \brief Set a function that will be called whenever samples are read
//  ///
//  /// The function func will be called whenever the low priority reader thread
//  /// reads samples from the audio file. The callback function will get the
//  /// samples that have just been read. This can be useful is the data is also
//  /// required by another thread than the audio thread. For example if you want
//  /// to display the audio data in addition to playing it. Bear in mind that if
//  /// the process taking place in the callback function is too intensive it
//  /// might produce an underrun in the ring buffer resulting in audio dropouts.
//  /// If this is the case, use the callback to copy the data to a separate
//  /// thread for processing.
//  ///
//  /// \param func the callback function
//  /// \param userData the data to be passed to the callback
//  ///
//  void setWriteCallback(CallbackFunc func, void *userData);

  int currentPosition();

protected:
  bool mRunning;
  float mMaxWriteTime {0};
//  std::atomic<int> mRepeats;
//  std::atomic<int> mSeek;
  std::atomic<int> mCurPos; // Updated once per read buffer
  std::mutex mLock;
  std::condition_variable mCondVar;
  std::thread *mReaderThread {nullptr};
  SingleRWRingBuffer *mRingBuffer {nullptr};
  uint32_t mBufferFrames;

  gam::SoundFile mSf;
//  CallbackFunc mReadCallback;
//  void *mCallbackData;

private:
  std::unique_ptr<float []> mFileBuffer; // Interleaved Buffer to copy file samples to (in the write thread before passing to ring buffer)

  static void writeFunction(SoundFileBufferedRecord *obj, std::condition_variable *cond, std::mutex *condMutex);
};

} // namespace al

#endif // SOUNDFILEBUFFERED_H
