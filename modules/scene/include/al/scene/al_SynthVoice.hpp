#ifndef AL_SYNTHVOICE_HPP
#define AL_SYNTHVOICE_HPP

/*	Allolib --
    Multimedia / virtual environment application class library

    Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012-2018. The Regents of the University of California.
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
    Polyphonic Synthesizer Voice

    File author(s):
    Andrés Cabrera mantaraya36@gmail.com
*/

#include <chrono>
#include <cstring>
#include <string>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "al/graphics/al_Graphics.hpp"
#include "al/io/al_AudioIOData.hpp"
#include "al/io/al_File.hpp"
#include "al/types/al_SingleRWRingBuffer.hpp"
#include "al/ui/al_Parameter.hpp"

namespace al {

/**
 * @brief The SynthVoice class
 * @ingroup Scene
 *
 * To create new voices for a PolySynth, inherit from this class and override
 * the onProcess() functions.
 * When inheriting this class you must provide a default construct that takes no
 * arguments.
 *
 * The "internal" parameters are a convenience function to save the user
 * management of parameters, so they are created and stored internally. The main
 * drawback is that they can only be "float" parameters. Internal parameters are
 * always "trigger" parameters. "Trigger" parameters registered with a voice
 * using SynthVoice::registerTriggerParameter(ParameterMeta &param)
 * SynthVoice::registerTriggerParameters(Args &...paramsArgs)
 * or the << operator.
 *
 * Trigger parameters are parameters that are meant to be set when the voice is
 * triggered, as their initial value needs to be set before any processing
 * begins, so their values will be stored and set when the note is triggered
 * from a SynthSequencer or when set by a remote node with DistributedScene (the
 * distributed version of PolySynth).
 *
 * When there is no need to set the parameters when triggering, you can register
 * parameters as regular/non-trigger parameters with registerParameter() or
 * registerParameters(). You will want to use these when you don't need to send
 * the initial values, as they are know at trigger time, and the values will be
 * updated continuously as the voice runs. Another reason to use these is for
 * parameter types that don't yet have support as trigger parameters, like
 * multivalue parameters like ParameterColor or ParameterVec.
 *
 */
class SynthVoice {
  friend class PolySynth; // PolySynth needs to access private members like
                          // "next".
public:
  SynthVoice() {}

  virtual ~SynthVoice() {}

  /// Returns true if voice is currently active
  bool active() { return mActive; }

  /**
     * @brief Set parameter values
     * @param pFields array containing the values
     * @param numFields number of fields to process
     * @return true if able to set the fields
     *
     * Note that this function allways triggers any associated callbacks, unlike
the other
+    * setTriggerParams() that take an argument determine this.
     */
  virtual bool setTriggerParams(float *pFields, int numFields = -1);

  /**
   * @brief Set trigger parameter values
   * @param pFields std::vector<float> containing the values
   * @return true if able to set the fields
   *
   * Trigger parameters are parameters meant to be set at triggering,
   * but that then stay constant throughout the duration of the instance.
   */
  virtual bool setTriggerParams(const std::vector<float> &pFields,
                                bool noCalls = false);

  /**
   * @brief Set parameter values
   * @param pFields std::vector<float> containing the values
   * @return true if able to set the fields
   */
  virtual bool setTriggerParams(const std::vector<VariantValue> &pFields,
                                bool noCalls = false);

  /**
   * @brief Get this instance's parameter fields
   * @param pFields a pre-allocated array where the parameter fields will be
   * written.
   * @param maxParams the maximum number of parameters to process (i.e. the
   * allocated size of pFields)
   * @return the number of parameters written
   * @ingroup Scene
   *
   * Copy the values from the internal parameters that have been
   * registered using registerParameterAsField or the << operator.
   */
  virtual int getTriggerParams(float *pFields, int maxParams = -1);

  /**
   * @brief Get this instance's parameter fields
   * @return pFields a pre-allocated array where the parameter fields will be
   * written.
   *
   * The default behavior is to copy the values from the internal parameters
   * that have been registered using registerParameterAsField or the <<
   * operator. Override this function in your voice if you need a different
   * behavior.
   */
  virtual std::vector<VariantValue> getTriggerParams();

  /**
   * @brief Override this function to define audio processing.
   * @param io
   *
   * You will need to mark this instance as done by calling the
   *  free() function when envelopes or processing is done. You should
   * call free() from one of the render() functions.
   */
  virtual void onProcess(AudioIOData & /*io*/) {}

  /**
   * @brief Override this function to define graphics for this synth
   */
  virtual void onProcess(Graphics & /*g*/) {}

  /**
   * @brief Override this function to update internal state, e.g. from an
   * asynchronous simulator
   *
   * dt is the delta time elapsed since last update
   */
  virtual void update(double dt = 0) { (void)dt; }

  /**
   * @brief Override this function to initialize internal data.
   *
   * This function should be called only once upon voice creation.
   */
  virtual void init() {}

  /**
   * @brief  Override this function to determine what needs to be done when
   * note/event starts.
   *
   * When a note starts, internal data within the algorithm usually needs to be
   * reset, e.g. reset envelopes, oscillator phase, etc.
   */
  virtual void onTriggerOn() {}

  /**
   * @brief  determine what needs to be done when note/event ends
   * Define this function to determine what needs to be done when note/event
   * ends. e.g. trigger release in envelopes, etc.
   * */
  virtual void onTriggerOff() { free(); }

  /**
   * @brief This function gets called after the voice is taken out of the
   * processing chain
   *
   * It can be used to store final states of a voice for example. This function
   * is currently called in the time master domain, so it might be important to
   * not do any blocking operations here.
   * */
  virtual void onFree() {}

  /**
   * @brief Trigger a note by calling onTriggerOn() and setting voice as active
   * @param offsetFrames
   *
   * This function can be called to programatically trigger a voice.
   * It is used for example in PolySynth to trigger a voice.
   */
  void triggerOn(int offsetFrames = 0);

  /**
   * @brief Call the voice's onTriggerOff() function to begin note's
   * deactivation
   * @param offsetFrames
   *
   * This function can be called to programatically trigger the release of a
   * voice
   */
  void triggerOff(int offsetFrames = 0);

  /**
   * @brief Set the id for this voice
   * @param idValue
   */
  void id(int idValue) { mId = idValue; }

  /**
   * @brief Get the id for this voice
   * @return
   */
  int id() { return mId; }

  /**
   * @brief returns the offset frames framesPerSecondand sets them to 0.
   * @param framesPerBuffer number of frames per buffer
   * @return offset frames
   *
   * Get the number of frames by which the start of this voice should be offset
   * within a processing block. This value is decremented by framesPerBuffer
   * once read.
   */
  int getStartOffsetFrames(unsigned int framesPerBuffer);

  int getEndOffsetFrames(unsigned int framesPerBuffer);

  void userData(void *ud) { mUserData = ud; }

  void *userData() { return mUserData; }

  /**
   * @brief Query the number of channels this voice generates
   * @return number of output channels
   *
   */
  unsigned int numOutChannels() { return mNumOutChannels; }

  /**
   * @brief A convenience function for quick creation of a managed parameter
   * @return a shared pointer to the created Parameter
   *
   * This creates a float type Parameter. The parameters instantiated
   * through this function can be queried and set quickly through
   * getInternalParameter(), getInternalParameterValue() and
   * setInternalParameterValue(). These functions provide a quick way to add
   * trigger parameters to the synth. These parameters are registered
   * automatically, so they will be used in automatic GUIs and with the
   * SynthSequencer.
   *
   * Internal parameters for voices are a convenience function to save the user
   * from managing parameters, but they are limited to float parameters only.
   */
  std::shared_ptr<Parameter>
  createInternalTriggerParameter(std::string name, float defaultValue = 0.0,
                                 float minValue = -9999.0,
                                 float maxValue = 9999.0);

  /**
   * @brief Get reference to internal Parameter
   * @param name parameter name
   * @return reference to the parameter
   *
   * Internal parameters are those registered through
   * createInternalTriggerParameter()
   */
  Parameter &getInternalParameter(std::string name);

  /**
   * @brief Get value for internal trigger parameter
   * @param name name of the parameter
   * @return current float value of the parameter
   *
   * Convenience function to get the value directly from an internal parameter
   * created by createInternalTriggerParameter()
   */
  float getInternalParameterValue(std::string name);

  /**
   * @brief Set value for internal trigger parameter
   * @param name name of the parameter
   * @param value new value for the parameter
   *
   * Convenience function to get the value directly from an internal parameter
   * created by createInternalTriggerParameter()
   */
  void setInternalParameterValue(std::string name, float value);

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
  virtual SynthVoice &registerTriggerParameter(ParameterMeta &param) {
    mTriggerParams.push_back(&param);
    return *this;
  }

  /**
   * Allows registering any number of trigger parameters on a single line
   */
  template <class... Args>
  SynthVoice &registerTriggerParameters(Args &...paramsArgs) {
    std::vector<ParameterMeta *> params{&paramsArgs...};
    for (auto *param : params) {
      registerTriggerParameter(*param);
    }
    return *this;
  }

  SynthVoice &operator<<(ParameterMeta &param) {
    return registerTriggerParameter(param);
  }

  std::vector<ParameterMeta *> triggerParameters() { return mTriggerParams; }

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
  virtual SynthVoice &registerParameter(ParameterMeta &param) {
    mContinuousParameters.push_back(&param);
    return *this;
  }

  /**
   * Allows registering any number of parameters on a single line
   */
  template <class... Args> SynthVoice &registerParameters(Args &...paramsArgs) {
    std::vector<ParameterMeta *> params{&paramsArgs...};
    for (auto *param : params) {
      registerParameter(*param);
    }
    return *this;
  }

  /**
   * @brief Get the Voice's continuous (i.e. not "trigger") parameters
   * @return vector with pointers to parameters
   */
  std::vector<ParameterMeta *> parameters() { return mContinuousParameters; }

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
  void free() { mActive = false; } // Mark this voice as done.

  /**
   * @brief Set voice as part of a replica distributed scene
   */
  void markAsReplica() { mIsReplica = true; }

  SynthVoice *next{nullptr}; // To support SynthVoices as linked lists

protected:
  /**
   * @brief Set the number of outputs this SynthVoice generates
   * @param numOutputs
   *
   * If you are using this voice within PolySynth, make sure this number is
   * less or equal than the number of output channels opened for the audio
   * device. If using in DynamicScene, make sure
   */
  void setNumOutChannels(unsigned int numOutputs) {
    mNumOutChannels = numOutputs;
  }

  std::vector<ParameterMeta *> mTriggerParams;

  std::vector<ParameterMeta *> mContinuousParameters;

  std::unordered_map<std::string, std::shared_ptr<Parameter>>
      mInternalParameters;

  bool mIsReplica{false}; // If voice is replica, it should not send its
                          // internal state but listen for changes.

private:
  int mId{-1};
  bool mActive{false};
  int mOnOffsetFrames{0};
  int mOffOffsetFrames{0};
  void *mUserData;
  unsigned int mNumOutChannels{1};
};

} // namespace al

#endif // AL_SYNTHVOICE_HPP
