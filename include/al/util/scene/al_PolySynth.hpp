#ifndef AL_POLYSYNTH_HPP
#define AL_POLYSYNTH_HPP

/*	Allolib --
    Multimedia / virtual environment application class library

    Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
    Copyright (C) 2012-2018. The Regents of the University of California.
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


#include <vector>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include <typeindex>

#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/io/al_AudioIOData.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/util/al_SingleRWRingBuffer.hpp"

namespace al
{

std::string demangle(const char* name); // Utility function.

int asciiToIndex(int asciiKey, int offset = 0);

int asciiToMIDI(int asciiKey, int offset = 0);

class ParameterField {
public:
  typedef enum {FLOAT, STRING} ParameterDataType;

  ParameterField(const float value) {
    mType = FLOAT;
    mData = new float;
    *static_cast<float *>(mData) = value;
  }

  ParameterField(const std::string value) {
    mType = STRING;
    mData = new std::string;
    *static_cast<std::string *>(mData) = value;
  }

  ParameterField(const char *value) {
    mType = STRING;
    mData = new std::string;
    *static_cast<std::string *>(mData) = value;
  }

  ParameterField(const ParameterField &paramField) : mType(paramField.mType){
    switch (mType) {
    case FLOAT:
      mData = new float;
      *static_cast<float *>(mData) = *static_cast<float *>(paramField.mData);
      break;
    case STRING:
      mData = new std::string;
      *static_cast<std::string *>(mData) = *static_cast<std::string *>(paramField.mData);
      break;
    }
  }

  virtual ~ParameterField() {
    switch (mType) {
    case FLOAT:
      delete static_cast<float *>(mData);
      break;
    case STRING:
      delete static_cast<std::string *>(mData);
      break;
    }
  }

  ParameterDataType type() {return mType;}

  //    float get() {
  //        assert(mType == FLOAT);
  //        return *static_cast<float *>(mData);
  //    }

  template<typename type>
  type get() {
    //        assert(mType == STRING);
    return *static_cast<type *>(mData);
  }

private:
  ParameterDataType mType;
  void *mData;
};

/**
 * @brief The SynthVoice class
 *
 * When inheriting this class you must provide a default construct that takes no arguments
*/
class SynthVoice {
  friend class PolySynth; // PolySynth needs to access private members like "next".
public:

  SynthVoice() {}

  virtual ~SynthVoice() {}

  /// Returns true if voice is currently active
  bool active() { return mActive;}

  /**
     * @brief Set parameter values
     * @param pFields array containing the values
     * @param numFields number of fields to process
     * @return true if able to set the fields
     *
     */
  virtual bool setTriggerParams(float *pFields, int numFields = -1) {
    if (numFields < (int) mTriggerParams.size()) {
      // std::cout << "Pfield size mismatch. Ignoring all." << std::endl;
      return false;
    }
    for (auto &param:mTriggerParams) {
      param->fromFloat(*pFields++);
    }
    return true;
  }

  /**
     * @brief Set trigger parameter values
     * @param pFields std::vector<float> containing the values
     * @return true if able to set the fields
     *
     * Trigger parameters are parameters meant to be set at triggering,
     * but that then stay constant throughout the duration of the instance.
     */
  virtual bool setTriggerParams(std::vector<float> &pFields) {
    if (pFields.size() < mTriggerParams.size()) {
      // std::cout << "pField count mismatch. Ignoring." << std::endl;
      return false;
    }
    auto it = pFields.begin();
    for (auto &param:mTriggerParams) {
      param->fromFloat(*it++);
    }
    return true;
  }

  /**
     * @brief Set parameter values
     * @param pFields std::vector<float> containing the values
     * @return true if able to set the fields
     */
  virtual bool setTriggerParams(std::vector<ParameterField> pFields) {
    if (pFields.size() < mTriggerParams.size()) {
      // std::cout << "pField count mismatch. Ignoring." << std::endl;
      return false;
    }
    auto it = pFields.begin();
    for (auto &param:mTriggerParams) {
      if (it->type() == ParameterField::FLOAT) {
        param->fromFloat(it->get<float>());
      } else if (it->type() == ParameterField::STRING) {
        if (strcmp(typeid(*param).name(), typeid(ParameterString).name() ) == 0) {
          static_cast<ParameterString *>(param)->set(it->get<std::string>());
        } else if (strcmp(typeid(*param).name(), typeid(ParameterMenu).name() ) == 0) {
          static_cast<ParameterMenu *>(param)->setCurrent(it->get<std::string>());
        } else {
          std::cerr << "ERROR: p-field string not setting ParameterString" << std::endl;
        }
      }
      it++;
    }
    return true;
  }

  /**
     * @brief Get this instance's parameter fields
     * @param pFields a pre-allocated array where the parameter fields will be written.
     * @param maxParams the maximum number of parameters to process (i.e. the allocated size of pFields)
     * @return the number of parameters written
     *
     * Copy the values from the internal parameters that have been
     * registered using registerParameterAsField or the << operator.
     */
  int getTriggerParams(float *pFields, int maxParams = -1) {
    std::vector<ParameterField> pFieldsVector = getTriggerParams();
    if (maxParams == -1) {
      assert(pFieldsVector.size() < INT_MAX);
      maxParams = int(pFieldsVector.size());
    }
    int count = 0;
    for (auto param: pFieldsVector) {
      if (count == maxParams) {
        break;
      }
      if (param.type() == ParameterField::FLOAT) {
        *pFields++ = param.get<float>();
      } else {
        *pFields++ = 0.0f; // Ignore strings...
      }
      count++;
    }
    return count;
  }

  /**
     * @brief Get this instance's parameter fields
     * @return pFields a pre-allocated array where the parameter fields will be written.
     *
     * The default behavior is to copy the values from the internal parameters that have been
     * registered using registerParameterAsField or the << operator. Override
     * this function in your voice if you need a different behavior.
     */
  virtual std::vector<ParameterField> getTriggerParams() {
    std::vector<ParameterField> pFields;
    pFields.reserve(mTriggerParams.size());
    for (auto param: mTriggerParams) {
      if (param) {
        if (strcmp(typeid(*param).name(), typeid(ParameterString).name() ) == 0) {
          pFields.push_back(static_cast<ParameterString *>(param)->get());
        } else if (strcmp(typeid(*param).name(), typeid(ParameterMenu).name() ) == 0) {
          pFields.push_back(static_cast<ParameterMenu *>(param)->getCurrent());
        } else {
          pFields.push_back(param->toFloat());
        }
      }
    }
    return pFields;
  }

  /**
     * @brief Override this function to define audio processing.
     * @param io
     *
     * You will need to mark this instance as done by calling the
     *  free() function when envelopes or processing is done. You should
     * call free() from one of the render() functions.
     */
  virtual void onProcess(AudioIOData& io) {}

  /**
     * @brief Override this function to define graphics for this synth
     */
  virtual void onProcess(Graphics &g) {}

  /**
     * @brief Override this function to update internal state, e.g. from an asynchronous simulator
     *
     * dt is the delta time elapsed since last update
     */
  virtual void update(double dt = 0) {}

  /**
     * @brief Override this function to initialize internal data.
     *
     * This function should be called only once upon voice creation.
     */
  virtual void init() {}

  /**
    * @brief  Override this function to determine what needs to be done when note/event starts.
    *
    * When a note starts, internal data within the algorithm usually needs to be reset,
    * e.g. reset envelopes, oscillator phase, etc.
    */
  virtual void onTriggerOn() {}

  /**
    * @brief  determine what needs to be done when note/event ends
    * Define this function to determine what needs to be done when note/event
    * ends. e.g. trigger release in envelopes, etc.
    * */
  virtual void onTriggerOff() {}

  /**
   * @brief Trigger a note by calling onTriggerOn() and setting voice as active
   * @param offsetFrames
   *
   * This function can be called to programatically trigger a voice.
   * It is used for example in PolySynth to trigger a voice.
   */
  void triggerOn(int offsetFrames = 0) {
    mOnOffsetFrames = offsetFrames;
    onTriggerOn();
    mActive = true;
  }

  /**
   * @brief Call the voice's onTriggerOff() function to begin note's deactivation
   * @param offsetFrames
   *
   * This function can be called to programatically trigger the release of a voice
   */
  void triggerOff(int offsetFrames = 0) {
    mOffOffsetFrames = offsetFrames; // TODO implement offset frames for trigger off. Currently ignoring and turning off at start of buffer
    onTriggerOff();
  }

  /**
   * @brief Set the id for this voice
   * @param idValue
   */
  void id(int idValue) {mId = idValue;}

  /**
   * @brief Get the id for this voice
   * @return
   */
  int id() {return mId;}

  /**
     * @brief returns the offset frames framesPerSecondand sets them to 0.
     * @param framesPerBuffer number of frames per buffer
     * @return offset frames
     *
     * Get the number of frames by which the start of this voice should be offset within a
     * processing block. This value is decremented by framesPerBuffer once read.
     */
  int getStartOffsetFrames(unsigned int framesPerBuffer);

  int getEndOffsetFrames(unsigned int framesPerBuffer);

  void userData(void *ud) {mUserData = ud;}

  void *userData() {return mUserData;}

  /**
   * @brief Query the number of channels this voice generates
   * @return number of output channels
   *
   */
  unsigned int numOutChannels() { return mNumOutChannels; }


  std::shared_ptr<Parameter> createInternalTriggerParameter(std::string name, float defaultValue = 0.0, float minValue = -9999.0, float maxValue = 9999.0) {
    mInternalParameters.push_back(std::make_shared<Parameter>(name, defaultValue, minValue, maxValue));
    registerTriggerParameter(*mInternalParameters.back().get());
    return mInternalParameters.back();
  }

  Parameter &getInternalParameter(std::string name) {
    for (auto param: mInternalParameters) {
      if (param->getName() == name &&
          strcmp(typeid(*param).name(), typeid(Parameter).name()) == 0) {
        return *param;
      }
    }
    std::cerr << "Parameter not found! Aborting: " << name << std::endl;
    throw "Invalid parameter name";
  }

  float getInternalParameterValue(std::string name) {
    for (auto param: mInternalParameters) {
      if (param->getName() == name) {
        return param->get();
      }
    }
    return 0.0;
  }

  void setInternalParameterValue(std::string name, float value) {
    for (auto param: mInternalParameters) {
      if (param->getName() == name) {
        param->set(value);
//        return;
      }
    }
  }

  /**
   * @brief Register a parameter as a "trigger" parameter
   * @param param
   * @return this SynthVoice object
   *
   * Trigger parameters are parameters meant to be set prior to triggering
   * the note and inserting it in the rendering chain. Trigger parameters
   * are garanteed to be set synchronously right before the note starts.
   * Additionally they are stored as values in a text file for the event
   * sequencer and are sent as part of the /triggerOn message when running
   * distributed.
   */
  virtual SynthVoice& registerTriggerParameter(ParameterMeta &param) { mTriggerParams.push_back(&param); return *this;}

  /**
   * Allows registering any number of trigger parameters on a single line
   */
  template<class... Args>
  SynthVoice& registerTriggerParameters(Args &... paramsArgs) {
      std::vector<ParameterMeta *> params{&paramsArgs...};
      for (auto *param: params) {
          registerTriggerParameter(*param);
      }
      return *this;
  }

  SynthVoice& operator<<(ParameterMeta &param) {return registerTriggerParameter(param);}

  std::vector<ParameterMeta *> triggerParameters() {return mTriggerParams;}

  /**
   * @brief registerParameter
   * @param param
   * @return
   *
   * Parameters are values that are meant to be updated while the voice is
   * running, as opposed to "trigger" parameters that are set at onTrigger().
   * "Trigger" parameters will be stored in sequence text files, while
   * regular parameters are meant to be changing within the voice.
   * In distributed scenes, to synchronize the internal values within voices,
   * the parameters must be registered through this function.
   */
  virtual SynthVoice& registerParameter(ParameterMeta &param) {
    mContinuousParameters.push_back(&param);
    return *this;
  }

  /**
   * Allows registering any number of parameters on a single line
   */
  template<class... Args>
  SynthVoice& registerParameters(Args &... paramsArgs) {
      std::vector<ParameterMeta *> params{&paramsArgs...};
      for (auto *param: params) {
          registerParameter(*param);
      }
      return *this;
  }

  std::vector<ParameterMeta *> parameters() {return mContinuousParameters;}

  /**
     * @brief Mark this voice as done.
     *
     * This should be set within one of the render()
     * functions when envelope or time is done and no more processing for
     * the note is needed. The voice will be considered ready for retriggering
     * by PolySynth.
     *
     * It can also be used to force removal of a voice from the rendering chain
     * without going through the release phase.
     */
  void free() {mActive = false; } // Mark this voice as done.

  SynthVoice *next {nullptr}; // To support SynthVoices as linked lists

protected:
  /**
     * @brief Set the number of outputs this SynthVoice generates
     * @param numOutputs
     *
     * If you are using this voice within PolySynth, make sure this number is
     * less or equal than the number of output channels opened for the audio
     * device. If using in DynamicScene, make sure
     */
  void setNumOutChannels(unsigned int numOutputs) {mNumOutChannels = numOutputs;}

  std::vector<ParameterMeta *> mTriggerParams;

  std::vector<ParameterMeta *> mContinuousParameters;

  std::vector<std::shared_ptr<Parameter>> mInternalParameters;
private:
  int mId {-1};
  int mActive {false};
  int mOnOffsetFrames {0};
  int mOffOffsetFrames {0};
  void *mUserData;
  unsigned int mNumOutChannels {1};
};

class PolySynth {
public:
  typedef enum {
    TIME_MASTER_AUDIO,
    TIME_MASTER_GRAPHICS,
    TIME_MASTER_ASYNC,
    TIME_MASTER_CPU
  } TimeMasterMode;

  friend class SynthSequencer;

  PolySynth(TimeMasterMode masterMode = TIME_MASTER_AUDIO);

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
  int triggerOn(SynthVoice *voice, int offsetFrames = 0, int id = -1, void *userData = nullptr);

  /// trigger release of voice with id
  void triggerOff(int id);

  /**
     * @brief Call TriggerOff for all active notes
     */
  virtual void allNotesOff();

  /**
     * @brief Get a reference to a voice.
     * @param forceAlloc force allocation of voice even if maximum allowed polyphony is reached
     * @return
     *
     * Returns a free voice from the internal dynamic allocated pool.
     * You must call triggerVoice to put the voice back in the rendering
     * chain after setting its properties, otherwise it will be lost.
     */
  template<class TSynthVoice>
  TSynthVoice *getVoice(bool forceAlloc = false);

  /**
     * @brief Get a reference to a free voice by voice type name
     * @param forceAlloc force allocation of voice even if maximum allowed polyphony is reached
     * @return
     *
     * Returns a free voice from the internal dynamic allocated pool.
     * If voice is not available. It will be allocated. Can return
     * nullptr if the class name and creator have not been registered
     * with registerSynthClass()
     */
  SynthVoice *getVoice(std::string name, bool forceAlloc = false);

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
     * Preallocate a number of voices of a particular TSynthVoice to avoid doing realtime
     * allocation.
     */
  template<class TSynthVoice>
  void allocatePolyphony(int number);

  /**
     * Preallocate a number of voices of a voice to avoid doing realtime
     * allocation. The name must be registered using registerSynthClass()
     */
  void allocatePolyphony(std::string name, int number);

  /**
     * @brief Use this function to insert a voice allocated externally into the free voice pool
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
  void setDefaultUserData(void *userData) { mDefaultUserData = userData;}

  /**
     * @brief Insert an AudioCallback object at the end of the callback queue
     * @param v The AudioCallback object
     * @return this instance
     *
     * The callback provided is appended to the list of post processing callbacks.
     * The post processing callbacks are run in sequence after the audio from all voices
     * has been rendered. This can be useful for global effects, spatialization, etc.
     */
  PolySynth &append(AudioCallback &v);

  /**
     *
     * @brief Insert an AudioCallback object at the head of the callback queue
     * @param v The AudioCallback object
     * @return this instance
     *
     * The callback provided is prepended to the list of post processing callbacks.
     * The post processing callbacks are run in sequence after the audio from all voices
     * has been rendered. This can be useful for global effects, spatialization, etc.
     */
  PolySynth &prepend(AudioCallback &v);

  /**
     *
     * @brief Insert an AudioCallback object before another in the queue
     * @param v The AudioCallback object
     * @param beforeThis The callback that will come after v
     * @return this instance
     *
     * The callback provided is inserted into the list of post processing callbacks.
     *
     * The post processing callbacks are run in sequence after the audio from all voices
     * has been rendered. This can be useful for global effects, spatialization, etc.
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
     * The callback provided is inserted into the list of post processing callbacks.
     *
     * The post processing callbacks are run in sequence after the audio from all voices
     * has been rendered. This can be useful for global effects, spatialization, etc.
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
     * The callback provided is removed from the list of post processing callbacks.
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
  void gain(float gainValue) {mAudioGain = gainValue;}

  /**
     * @brief get current audio gain
     * @return
     */
  float gain() {return mAudioGain;}

  /**
     * @brief register a callback to be notified of a trigger on event
     */
  void registerTriggerOnCallback(std::function<bool(SynthVoice *voice, int offsetFrames, int id, void *userData)> cb, void *userData = nullptr) {
    TriggerOnCallback cbNode(cb, userData);
    mTriggerOnCallbacks.push_back(cbNode);
  }

  /**
     * @brief register a callback to be notified of a trigger off event
     */
  void registerTriggerOffCallback(std::function<bool(int id, void *userData)> cb, void *userData = nullptr) {
    TriggerOffCallback cbNode(cb, userData);
    mTriggerOffCallbacks.push_back(cbNode);
  }

  /**
     * @brief register a callback to be notified of allocation of a voice.
     */
  void registerAllocateCallback(std::function<void(SynthVoice *voice, void *)> cb, void *userData = nullptr) {
    AllocationCallback cbNode(cb, userData);
    mAllocationCallbacks.push_back(cbNode);
  }

  /**
     * Register a SynthVoice class to allow instantiating it by name
     *
     * This is needed for remote instantiation and for text sequence playback.
     */
  template<class TSynthVoice>
  void registerSynthClass(std::string name = "", bool allowAutoAllocation = true) {
    if (name.size() == 0) {
      name = demangle(typeid(TSynthVoice).name());
    }
    if (mCreators.find(name) != mCreators.end()) {
      if (mVerbose) {
        std::cout << "Warning: Overriding registration of SynthVoice: " << name << std::endl;
      }
    }
    if (!allowAutoAllocation) {
      if (std::find(mNoAllocationList.begin(), mNoAllocationList.end(), name) == mNoAllocationList.end()) {
        mNoAllocationList.push_back(name);
      }
    }
    mCreators[name] = [&]() {
      TSynthVoice *voice = allocateVoice<TSynthVoice>();
      return voice;
    };
  }

  SynthVoice *allocateVoice(std::string name);

  template<class TSynthVoice>
  TSynthVoice *allocateVoice() {
    TSynthVoice *voice = new TSynthVoice;
    for(auto allocCb: mAllocationCallbacks) {
      allocCb.first(voice, allocCb.second);
    }
    if(mDefaultUserData) {
      voice->userData(mDefaultUserData);
    }
    voice->init();
    return voice;
  }

  bool verbose() { return mVerbose; }
  void verbose(bool verbose) { mVerbose = verbose; }

  // Use this function with care as there are no memory protections
  SynthVoice *getActiveVoices() {
    return mActiveVoices;
  }

  void setCpuClockGranularity(double timeSecs) {
    mCpuGranularitySec = timeSecs;
  }

protected:
  inline void processVoices() {
    if (mVoiceToInsertLock.try_lock()) {
      if (mVoicesToInsert) {
        // If lock acquired insert queued voices
        if (mActiveVoices) {

          auto voice = mVoicesToInsert;
          while (voice->next) { // Find last voice to insert
            voice = voice->next;
          }
          voice->next = mActiveVoices; // Connect last inserted to previously active
          mActiveVoices = mVoicesToInsert; // Put new voices in head
        } else {
          mActiveVoices = mVoicesToInsert;
        }
        if (verbose()) {
          std::cout << "Voice on "<<  mVoicesToInsert->id() << std::endl;
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
          while(voice) {
            voice->id(-1);
            previousVoice = voice;
            voice = voice->next;
          }
          previousVoice->next = mFreeVoices; // Connect last active voice to first free voice
          mFreeVoices = mActiveVoices; // Move all voices to free voices
          mActiveVoices = nullptr; // No active voices left
        }
        mFreeVoiceLock.unlock();
      }
    }
  }

  inline void processVoiceTurnOff() {
    int voicesToTurnOff[16];
    size_t numVoicesToTurnOff;
    while ( (numVoicesToTurnOff = mVoiceIdsToTurnOff.read((char *) voicesToTurnOff, 16 * sizeof (int))) ) {
      for (size_t i = 0; i < numVoicesToTurnOff/int(sizeof (int)); i++) {
        auto voice = mActiveVoices;
        while (voice) {
          if (voice->id() == voicesToTurnOff[i]) {

            if (mVerbose) {
              std::cout << "Voice off "<<  voice->id() << std::endl;
            }
            voice->triggerOff(); // TODO use offset for turn off
          }
          voice = voice->next;
        }
      }
    }
  }

  inline void processInactiveVoices() {
    // Move inactive voices to free queue
    if (mFreeVoiceLock.try_lock()) { // Attempt to remove inactive voices without waiting.
      auto voice = mActiveVoices;
      SynthVoice *previousVoice = nullptr;
      while(voice) {
        if (!voice->active()) {
          if (previousVoice) {
            previousVoice->next = voice->next; // Remove from active list
            voice->next = mFreeVoices;
            mFreeVoices = voice; // Insert as head in free voices
            voice->id(-1); // Reset voice id
            voice = previousVoice; // prepare next iteration
          } else { // Inactive is head of the list
            mActiveVoices = voice->next; // Remove voice from list
            voice->next = mFreeVoices;
            mFreeVoices = voice; // Insert as head in free voices
            voice->id(-1); // Reset voice id
            voice = mActiveVoices; // prepare next iteration
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

  inline void processGain(AudioIOData &io) {
    io.frame(0);
    if (mAudioGain != 1.0f) {
      for (unsigned int i = 0; i < io.channelsOut(); i++) {
        float *buffer = io.outBuffer(i);
        unsigned int samps = io.framesPerBuffer();
        while (samps--) {
          *buffer++ *= mAudioGain;
        }
      }
    }
  }

  // Internal voices are allocated in PolySynth and shared with the outside.
  SynthVoice *mVoicesToInsert {nullptr}; //Voices to be inserted in the realtime context
  SynthVoice *mFreeVoices {nullptr}; // Allocated voices available for reuse
  SynthVoice *mActiveVoices {nullptr}; // Dynamic voices that are currently active. Only modified within the master domain (set by mMasterMode)
  std::mutex mVoiceToInsertLock;
  std::mutex mFreeVoiceLock;
  std::mutex mGraphicsLock;

  SingleRWRingBuffer mVoiceIdsToTurnOff {64 * sizeof(int)};

  TimeMasterMode mMasterMode;

  std::vector<AudioCallback *> mPostProcessing;

  typedef std::pair<std::function<bool(SynthVoice *voice, int offsetFrames, int id, void *)>, void *> TriggerOnCallback;
  std::vector<TriggerOnCallback> mTriggerOnCallbacks;

  typedef std::pair<std::function<bool(int id, void *)>, void *> TriggerOffCallback;
  std::vector<TriggerOffCallback> mTriggerOffCallbacks;

  typedef std::pair<std::function<void(SynthVoice *voice, void *)>, void *> AllocationCallback;

  std::vector<AllocationCallback> mAllocationCallbacks;

  float mAudioGain {1.0f};

  int mIdCounter {0};

  bool mAllNotesOff {false}; // Flag used to notify processing to turn off all voices

  typedef std::function<SynthVoice *()> VoiceCreatorFunc;
  typedef std::map<std::string, VoiceCreatorFunc> Creators;

  void *mDefaultUserData {nullptr};

  Creators mCreators;
  std::vector<std::string> mNoAllocationList; // Disallow auto allocation for class name. Set in allocateVoice()

  bool mRunCPUClock {true};
  double mCpuGranularitySec = 0.001; // 1ms
  std::unique_ptr<std::thread> mCpuClockThread;

  bool mVerbose {false};
};

template<class TSynthVoice>
TSynthVoice *PolySynth::getVoice(bool forceAlloc) {
    std::unique_lock<std::mutex> lk(mFreeVoiceLock); // Only one getVoice() call at a time
    SynthVoice *freeVoice = mFreeVoices;
    SynthVoice *previousVoice = nullptr;
    if (forceAlloc) {
      freeVoice = nullptr;
    } else {
      while (freeVoice) {
          if (std::type_index(typeid(*freeVoice)) == std::type_index(typeid(TSynthVoice))) {
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

      if (mVerbose) {
        std::cout << "Allocating voice of type " << typeid (TSynthVoice).name() << "." << std::endl;
      }
      freeVoice = allocateVoice<TSynthVoice>();
    }
    return static_cast<TSynthVoice *>(freeVoice);
}

template<class TSynthVoice>
void PolySynth::allocatePolyphony(int number) {
    std::unique_lock<std::mutex> lk(mFreeVoiceLock);
    SynthVoice *lastVoice = mFreeVoices;
    if (lastVoice) {
        while (lastVoice->next) { lastVoice = lastVoice->next; }
    } else {
        lastVoice = mFreeVoices = allocateVoice<TSynthVoice>();
        number--;
    }
    for(int i = 0; i < number; i++) {
        lastVoice->next = allocateVoice<TSynthVoice>();
        lastVoice = lastVoice->next;
    }
}

} // namespace al

#endif  // AL_POLYSYNTH_HPP
