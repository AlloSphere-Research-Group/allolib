#ifndef AL_POLYSYNTH_HPP
#define AL_POLYSYNTH_HPP

/*	Allolib --
    Multimedia / virtual environment application class library

    Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012-2022. The Regents of the University of California.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the University of California nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    File description:
    Polyphonic Synthesizer

    File author(s):
    Andrés Cabrera mantaraya36@gmail.com
*/

#include <chrono>
#include <cstring>
#include <string>
#include <thread>
#include <typeindex>
#include <vector>

#include "al/graphics/al_Graphics.hpp"
#include "al/io/al_AudioIOData.hpp"
#include "al/io/al_File.hpp"
#include "al/scene/al_SynthVoice.hpp"
#include "al/types/al_SingleRWRingBuffer.hpp"
#include "al/ui/al_Parameter.hpp"

namespace al {

int asciiToIndex(int asciiKey, int offset = 0);

int asciiToMIDI(int asciiKey, int offset = 0);

/**
 * @brief A PolySynth manages polyphony and rendering of SynthVoice instances.
 * @ingroup Scene
 *
 */
class PolySynth {
public:
  friend class SynthSequencer;

  PolySynth(TimeMasterMode masterMode = TimeMasterMode::TIME_MASTER_AUDIO);

  virtual ~PolySynth();

  /**
   * @brief trigger Puts voice in active voice lit and calls triggerOn() for it
   * @param voice pointer to the voice to trigger
   * @return a unique id for the voice
   *
   * You can use the id to identify the note for later triggerOff() calls.
   * Always use a positive value for id, as negative ids have special treatment
   * in classes like this one and SynthGUIManager
   */
  int triggerOn(SynthVoice *voice, int offsetFrames = 0, int id = -1,
                void *userData = nullptr);

  /// trigger release of voice with id
  void triggerOff(int id);

  /**
   * @brief Turn off all notes immediately (without calling triggerOff() )
   */
  virtual void allNotesOff();

  /**
   * @brief Get a reference to a voice.
   * @param forceAlloc force allocation of voice even if maximum allowed
   * polyphony is reached
   * @return
   *
   * Returns a free voice from the internal dynamic allocated pool.
   * You must call triggerVoice to put the voice back in the rendering
   * chain after setting its properties, otherwise it will be lost.
   */
  template <class TSynthVoice> TSynthVoice *getVoice(bool forceAlloc = false);

  /**
   * @brief Get a reference to a free voice by voice type name
   * @param forceAlloc force allocation of voice even if maximum allowed
   * polyphony is reached
   * @return
   *
   * Returns a free voice from the internal dynamic allocated pool.
   * If voice is not available, it will be allocated if disableAllocation() has
   * not been called. Can return nullptr if the class name and creator have not
   * been registered with registerSynthClass()
   */
  /*[[nodiscard]]*/ SynthVoice *getVoice(std::string name,
                                         bool forceAlloc = false);

  /**
   * @brief Get the first available voice with minimal checks
   * @return
   *
   * This is a quick function with little overhead for PolySynths that handle
   * only one type of voice.
   */
  /*[[nodiscard]]*/ SynthVoice *getFreeVoice();

  /**
   * @brief render all the active voices into the audio buffers
   * @param io AudioIOData containing buffers and audio I/O meta data
   */
  virtual void render(AudioIOData &io);

  /**
   * @brief render graphics for all active voices
   */
  virtual void render(Graphics &g);

  /**
   * @brief update internal state for all voices.
   */
  virtual void update(double dt = 0);

  /**
   * Preallocate a number of voices of a particular TSynthVoice to avoid doing
   * realtime allocation.
   */
  template <class TSynthVoice> void allocatePolyphony(int number);

  template <class TSynthVoice> void disableAllocation();

  void disableAllocation(std::string name);

  /**
   * Preallocate a number of voices of a voice to avoid doing realtime
   * allocation. The name must be registered using registerSynthClass()
   */
  void allocatePolyphony(std::string name, int number);

  /**
   * @brief Use this function to insert a voice allocated externally into the
   * free voice pool
   * @param voice the new voice to be added to the free voice pool
   *
   * You can also use this function to return to the polysynth a voice
   * requested that will not be used.
   */
  void insertFreeVoice(SynthVoice *voice);

  /**
   * @brief Remove voice from the free voice pool
   * @param voice
   */
  bool popFreeVoice(SynthVoice *voice);

  /**
   * @brief Set default user data to set to voices before the are returned
   * by getVoice()
   * @param userData
   */
  void setDefaultUserData(void *userData) { mDefaultUserData = userData; }

  /**
   * @brief Set time master context
   * @param master domain
   *
   * You should not call this function if any of the rendering domains are
   * running.
   */
  void setTimeMaster(TimeMasterMode masterMode);

  /**
   * @brief Insert an AudioCallback object at the end of the callback queue
   * @param v The AudioCallback object
   * @return this instance
   *
   * The callback provided is appended to the list of post processing callbacks.
   * The post processing callbacks are run in sequence after the audio from all
   * voices has been rendered. This can be useful for global effects,
   * spatialization, etc.
   */
  PolySynth &append(AudioCallback &v);

  /**
   *
   * @brief Insert an AudioCallback object at the head of the callback queue
   * @param v The AudioCallback object
   * @return this instance
   *
   * The callback provided is prepended to the list of post processing
   * callbacks. The post processing callbacks are run in sequence after the
   * audio from all voices has been rendered. This can be useful for global
   * effects, spatialization, etc.
   */
  PolySynth &prepend(AudioCallback &v);

  /**
   *
   * @brief Insert an AudioCallback object before another in the queue
   * @param v The AudioCallback object
   * @param beforeThis The callback that will come after v
   * @return this instance
   *
   * The callback provided is inserted into the list of post processing
   * callbacks.
   *
   * The post processing callbacks are run in sequence after the audio from all
   * voices has been rendered. This can be useful for global effects,
   * spatialization, etc.
   *
   * If beforeThis is not found v is prepended to the callback list
   */
  PolySynth &insertBefore(AudioCallback &v, AudioCallback &beforeThis);

  /**
   *
   * @brief Insert an AudioCallback object after another in the queue
   * @param v The AudioCallback object
   * @param beforeThis The callback that will come before v
   * @return this instance
   *
   * The callback provided is inserted into the list of post processing
   * callbacks.
   *
   * The post processing callbacks are run in sequence after the audio from all
   * voices has been rendered. This can be useful for global effects,
   * spatialization, etc.
   *
   * If afterThis is not found v is prepended to the callback list
   */
  PolySynth &insertAfter(AudioCallback &v, AudioCallback &afterThis);

  /**
   *
   * @brief Remove an AudioCallback object from the queue
   * @param v The AudioCallback object
   * @return this instance
   *
   * The callback provided is removed from the list of post processing
   * callbacks.
   *
   * If v is not found, this call has no effect.
   */
  PolySynth &remove(AudioCallback &v);

  /**
   * @brief prints details of the allocated voices (free, active and queued)
   *
   * Warning: this function is not thread safe and might crash if the audio
   * or graphics is running. Only use for temporary debugging.
   */
  void print(std::ostream &stream = std::cout);

  /**
   * @brief Set audio output gain
   * @param gainValue
   */
  void gain(float gainValue) { mAudioGain = gainValue; }

  /**
   * @brief get current audio gain
   * @return
   */
  float gain() { return mAudioGain; }

  /**
   * @brief register a callback to be notified of a trigger on event
   */
  void registerTriggerOnCallback(
      std::function<bool(SynthVoice *voice, int offsetFrames, int id,
                         void *userData)>
          cb,
      void *userData = nullptr);

  /**
   * @brief register a callback to be notified of a trigger off event
   */
  void
  registerTriggerOffCallback(std::function<bool(int id, void *userData)> cb,
                             void *userData = nullptr);
  /**
   * @brief register a callback to be notified of a note is moved to the free
   * pool from the active list
   */
  void registerFreeCallback(std::function<bool(int id, void *userData)> cb,
                            void *userData = nullptr);

  /**
   * @brief register a callback to be notified of allocation of a voice.
   */
  void
  registerAllocateCallback(std::function<void(SynthVoice *voice, void *)> cb,
                           void *userData = nullptr);

  /**
   * Register a SynthVoice class to allow instantiating it by name
   *
   * This is needed for remote instantiation and for text sequence playback.
   */
  template <class TSynthVoice>
  void registerSynthClass(std::string name = "",
                          bool allowAutoAllocation = true) {
    if (name.size() == 0) {
      name = demangle(typeid(TSynthVoice).name());
    }
    if (mCreators.find(name) != mCreators.end()) {
      if (mVerbose) {
        std::cout << "Warning: Overriding registration of SynthVoice: " << name
                  << std::endl;
      }
    }
    if (!allowAutoAllocation) {
      if (std::find(mNoAllocationList.begin(), mNoAllocationList.end(), name) ==
          mNoAllocationList.end()) {
        mNoAllocationList.push_back(name);
      }
    }
    mCreators[name] = [&]() {
      TSynthVoice *voice = allocateVoice<TSynthVoice>();
      return voice;
    };
  }

  SynthVoice *allocateVoice(std::string name);

  template <class TSynthVoice> TSynthVoice *allocateVoice() {
    TSynthVoice *voice = new TSynthVoice;
    voice->next = nullptr;
    if (mDefaultUserData) {
      voice->userData(mDefaultUserData);
    }
    voice->init();
    for (auto allocCb : mAllocationCallbacks) {
      allocCb.first(voice, allocCb.second);
    }
    return voice;
  }

  bool verbose() { return mVerbose; }
  void verbose(bool verbose) { mVerbose = verbose; }

  /**
   * @brief getActiveVoices
   * @return
   *
   * This function must be used with care as there is no guarantee that the
   * list is not modified while it is being used. You should only use this
   * function in the same context determined by the time master setting.
   * For example, if TimeMasterMode::TIME_MASTER_AUDIO you should only call
   * this function within the audio context (i.e. the context that calls
   * render(AudioIOData &io) )
   */
  SynthVoice *getActiveVoices() { return mActiveVoices; }

  /**
   * @brief getFreeVoices
   * @return
   *
   * This function is unsafe and should be used with extreme care. Ensure that
   * no allocation, voice insertion or removal takes place while working with
   * these voices.
   */
  SynthVoice *getFreeVoices() { return mFreeVoices; }

  /**
   * @brief Determines the number of output channels allocated for the internal
   * AudioIOData objects
   * @param channels
   *
   * Always call prepare() after calling this function. The changes are only
   * applied by prepare().
   */
  void setVoiceMaxOutputChannels(uint16_t channels);

  /**
   * @brief Determines the number of buses for the internal AudioIOData objects
   * @param channels
   *
   * Always call prepare() after calling this function. The changes are only
   * applied by prepare().
   */
  void setVoiceBusChannels(uint16_t channels) { mVoiceBusChannels = channels; }

  typedef const std::function<void(AudioIOData &internalVoiceIO,
                                   Pose &channelPose)>
      BusRoutingCallback;

  /**
   * @brief setBusRoutingCallback
   * @param cb
   *
   * This function will be called after all voices have rendered their output
   * and prior to the function call to process spatialization. Can be used to
   * route signals to buses.
   */
  void setBusRoutingCallback(BusRoutingCallback cb);

  /**
   * @brief Set channel map for output.
   * @param channelMap
   *
   * The index into the vector refers to the output channel in the voice buffer,
   * the value is the channel index to write for audio output. This operation
   * should be performed only when audio is not running. Channel mapping is
   * ignored in DynamicScene and DistributedScene.
   */
  void setChannelMap(std::vector<size_t> channelMap);

  /**
   * @brief Set the time in seconds to wait between sequencer updates when time
   * master is CPU.
   *
   * This has no effect if time master mode is not TIME_MASTER_CPU
   */
  void setCpuClockGranularity(double timeSecs) {
    mCpuGranularitySec = timeSecs;
  }

  /**
   * @brief Add new voices to the chain.
   *
   * You need to call this function only if you are in TIME_MASTER_FREE mode.
   * In other modes it is called in the render() function for the domain.
   */
  inline void processVoices() {
    if (mVoiceToInsertLock.try_lock()) {
      if (mVoicesToInsert) {
        // If lock acquired insert queued voices
        if (mActiveVoices) {
          auto voice = mVoicesToInsert;
          while (voice->next) { // Find last voice to insert
            voice = voice->next;
          }
          voice->next =
              mActiveVoices; // Connect last inserted to previously active
          mActiveVoices = mVoicesToInsert; // Put new voices in head
        } else {
          mActiveVoices = mVoicesToInsert;
        }
        if (verbose()) {
          std::cout << "Voice on " << mVoicesToInsert->id() << std::endl;
        }

        mVoicesToInsert = nullptr;
      }
      mVoiceToInsertLock.unlock();
    }
    if (mAllNotesOff) {
      if (mFreeVoiceLock.try_lock()) {
        mAllNotesOff = false;
        if (mActiveVoices) {
          auto voice = mActiveVoices->next;
          SynthVoice *previousVoice = mActiveVoices;
          previousVoice->id(-1);
          while (voice) {
            voice->id(-1);
            previousVoice = voice;
            voice = voice->next;
          }
          previousVoice->next =
              mFreeVoices; // Connect last active voice to first free voice
          mFreeVoices = mActiveVoices; // Move all voices to free voices
          mActiveVoices = nullptr;     // No active voices left
        }
        mFreeVoiceLock.unlock();
      }
    }
  }

  /**
   * @brief Check for voices that need trigger off and execute
   *
   * You need to call this function only if you are in TIME_MASTER_FREE mode.
   * In other modes it is called in the render() function for the domain.
   */
  inline void processVoiceTurnOff() {
    int voicesToTurnOff[16];
    size_t numVoicesToTurnOff;
    while ((numVoicesToTurnOff = mVoiceIdsToTurnOff.read(
                (char *)voicesToTurnOff, 16 * sizeof(int)))) {
      for (size_t i = 0; i < numVoicesToTurnOff / int(sizeof(int)); i++) {
        auto *voice = mActiveVoices;
        while (voice) {
          if (voice->id() == voicesToTurnOff[i]) {
            if (mVerbose) {
              std::cout << "Voice trigger off " << voice->id() << std::endl;
            }
            voice->triggerOff(); // TODO use offset for turn off
          }
          voice = voice->next;
        }
      }
    }
    size_t numVoicesToFree;
    int voicesToFree[16];
    while ((numVoicesToFree =
                mVoiceIdsToFree.read((char *)voicesToFree, 16 * sizeof(int)))) {
      for (size_t i = 0; i < numVoicesToFree / int(sizeof(int)); i++) {
        if (mVerbose) {
          std::cout << "Voice free " << voicesToFree[i] << std::endl;
        }
        auto *voice = mActiveVoices;
        while (voice) {
          if (voice->id() == voicesToFree[i]) {
            voice->mActive = false;
          }
          voice = voice->next;
        }
      }
    }
  }

  /**
   * @brief Check for voices marked as free and move them to the free voice pool
   *
   * You need to call this function only if you are in TIME_MASTER_FREE mode.
   * In other modes it is called in the render() function for the domain.
   */
  inline void processInactiveVoices() {
    // Move inactive voices to free queue
    if (mFreeVoiceLock.try_lock()) { // Attempt to remove inactive voices
      // without waiting.
      auto *voice = mActiveVoices;
      SynthVoice *previousVoice = nullptr;
      while (voice) {
        if (!voice->active()) {
          int id = voice->id();
          //          std::cout << " ****((((())))) **** Remove inactive voice:
          //          " << voice << std::endl;
          if (previousVoice) {
            previousVoice->next = voice->next; // Remove from active list
            voice->next = mFreeVoices;
            mFreeVoices = voice; // Insert as head in free voices
            voice->id(-1);       // Reset voice id
            voice->onFree();
            voice = previousVoice; // prepare next iteration
          } else {                 // Inactive is head of the list
            auto *nextVoice = voice->next;
            mActiveVoices = nextVoice; // Remove voice from list
            voice->next = mFreeVoices;
            mFreeVoices = voice; // Insert as head in free voices
            voice->id(-1);       // Reset voice id
            voice->onFree();
            voice = voice->next; // prepare next iteration
          }
          for (const auto &cbNode : mFreeCallbacks) {
            cbNode.first(id, cbNode.second);
          }
        }
        previousVoice = voice;
        if (voice) {
          voice = voice->next;
        }
      }
      mFreeVoiceLock.unlock();
    }
  }

protected:
  void startCpuClockThread();

  inline void processGain(AudioIOData &io) {
    io.frame(0);
    if (mAudioGain != 1.0f) {
      for (unsigned int i = 0; i < io.channelsOut(); i++) {
        float *buffer = io.outBuffer(i);
        uint64_t samps = io.framesPerBuffer();
        while (samps--) {
          *buffer++ *= mAudioGain;
        }
      }
    }
  }

  virtual void prepare(AudioIOData &io);

  /// Voices to be inserted in the realtime context. Internal voices are
  /// allocated in PolySynth and shared with the outside.
  SynthVoice *mVoicesToInsert{nullptr};
  /// Allocated voices available for reuse
  SynthVoice *mFreeVoices{nullptr};
  /// Dynamic voices that are currently active. Only modified
  /// within the master domain (set by mMasterMode)
  SynthVoice *mActiveVoices{nullptr};
  std::mutex mVoiceToInsertLock;
  std::mutex mFreeVoiceLock;
  std::mutex mGraphicsLock; // TODO: remove this lock?

  bool m_useInternalAudioIO = true;
  bool m_internalAudioConfigured = false;

  uint16_t mVoiceMaxOutputChannels = 2;
  uint16_t mVoiceMaxInputChannels = 0;
  uint16_t mVoiceBusChannels = 0;
  std::shared_ptr<BusRoutingCallback> mBusRoutingCallback;
  AudioIOData internalAudioIO;

  SingleRWRingBuffer mVoiceIdsToTurnOff{64 * sizeof(int)};
  SingleRWRingBuffer mVoiceIdsToFree{64 * sizeof(int)};

  TimeMasterMode mMasterMode;

  std::vector<AudioCallback *> mPostProcessing;

  using TriggerOnCallback = std::pair<
      std::function<bool(SynthVoice *voice, int offsetFrames, int id, void *)>,
      void *>;
  std::vector<TriggerOnCallback> mTriggerOnCallbacks;

  using TriggerOffCallback =
      std::pair<std::function<bool(int id, void *)>, void *>;
  std::vector<TriggerOffCallback> mTriggerOffCallbacks;

  using FreeCallback = std::pair<std::function<bool(int id, void *)>, void *>;
  std::vector<FreeCallback> mFreeCallbacks;

  using AllocationCallback =
      std::pair<std::function<void(SynthVoice *voice, void *)>, void *>;
  std::vector<AllocationCallback> mAllocationCallbacks;

  float mAudioGain{1.0f};

  int mIdCounter{1000};

  // Flag used to notify processing to turn off all voices
  bool mAllNotesOff{false};

  typedef std::function<SynthVoice *()> VoiceCreatorFunc;
  typedef std::map<std::string, VoiceCreatorFunc> Creators;

  void *mDefaultUserData{nullptr};

  Creators mCreators;
  // Disallow auto allocation for class name. Set in allocateVoice()
  std::vector<std::string> mNoAllocationList;
  std::vector<size_t> mChannelMap; // Maps synth output to audio channels

  bool mRunCPUClock{true};
  double mCpuGranularitySec = 0.001; // 1ms
  std::unique_ptr<std::thread> mCpuClockThread;

  bool mVerbose{false};
};

template <class TSynthVoice> void PolySynth::disableAllocation() {
  std::string name = demangle(typeid(TSynthVoice).name());
  disableAllocation(name);
}

template <class TSynthVoice> TSynthVoice *PolySynth::getVoice(bool forceAlloc) {
  std::unique_lock<std::mutex> lk(
      mFreeVoiceLock); // Only one getVoice() call at a time
  SynthVoice *freeVoice = mFreeVoices;
  SynthVoice *previousVoice = nullptr;
  if (forceAlloc) {
    freeVoice = nullptr;
  } else {
    while (freeVoice) {
      if (std::type_index(typeid(*freeVoice)) ==
          std::type_index(typeid(TSynthVoice))) {
        if (previousVoice) {
          previousVoice->next = freeVoice->next;
        } else {
          mFreeVoices = freeVoice->next;
        }
        break;
      }
      previousVoice = freeVoice;
      freeVoice = freeVoice->next;
    }
  }
  if (!freeVoice) { // No free voice in list, so we need to allocate it
    // TODO report current polyphony for more informed allocation of polyphony
    // TODO check if allocation allowed
    //  But only allocate if allocation has not been disabled
    std::string name = demangle(typeid(TSynthVoice).name());
    if (std::find(mNoAllocationList.begin(), mNoAllocationList.end(), name) ==
        mNoAllocationList.end()) {
      // TODO report current polyphony for more informed allocation of polyphony
      freeVoice = allocateVoice<TSynthVoice>();
      if (mVerbose) {
        std::cout << "Allocating voice of type " << typeid(TSynthVoice).name()
                  << "." << std::endl;
      }
    } else {
      std::cout << "Automatic allocation disabled for voice:" << name
                << std::endl;
    }
  }
  return static_cast<TSynthVoice *>(freeVoice);
}

template <class TSynthVoice> void PolySynth::allocatePolyphony(int number) {
  std::unique_lock<std::mutex> lk(mFreeVoiceLock);
  SynthVoice *lastVoice = mFreeVoices;
  if (lastVoice) {
    while (lastVoice->next) {
      lastVoice = lastVoice->next;
    }
  } else {
    lastVoice = mFreeVoices = allocateVoice<TSynthVoice>();
    number--;
  }
  for (int i = 0; i < number; i++) {
    lastVoice->next = allocateVoice<TSynthVoice>();
    lastVoice = lastVoice->next;
  }
}

} // namespace al

#endif // AL_POLYSYNTH_HPP
