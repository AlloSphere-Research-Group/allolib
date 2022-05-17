#ifndef AL_PRESETSEQUENCER_H
#define AL_PRESETSEQUENCER_H

/*	Allocore --
        Multimedia / virtual environment application class library

        Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012-2016. The Regents of the University of California.
        All rights reserved.

        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions are
   met:

                Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

                Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer in the
                documentation and/or other materials provided with the
   distribution.

                Neither the name of the University of California nor the names
   of its contributors may be used to endorse or promote products derived from
                this software without specific prior written permission.

        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
        IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

        File description:
        Preset sequencing

        File author(s):
        Andrés Cabrera mantaraya36@gmail.com
*/

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>

#include "al/protocol/al_OSC.hpp"
#include "al/ui/al_PresetHandler.hpp"

namespace al {

class SequenceRecorder;
class Composition;

/**
 * @brief The PresetSequencer class allows triggering presets from a
 * PresetHandler over time.
 * @ingroup UI
 *
 * In order to use a preset sequencer you must register a preset handler with
 * the sequencer:
 *
 * @code
 * PresetHandler presetHandler;
 * PresetSequencer sequencer;
 *
 * sequencer << presetHandler;
 * @endcode
 *
 * Sequences that trigger presets are stored in text files with the format:
 *
 * @code
 * preset1:0.0:3.0
 * preset2:4.0:2.0
 * preset3:1.0:1.0
 * ::
 * @endcode
 *
 * The first element in each line specifies the preset name that the
 * PresetHandler should load (i.e. a file called "preset1.preset" in the current
 * preset directory, see PresetHandler::getCurrentPath() ). The second element
 * determines the time to get to the preset from the current state, i.e. the
 * "morph time" to reach the preset. The third element determines the time the
 * preset should be held after reaching it, the "wait time".
 *
 * The file should end with two colons (::). Everything after this will be
 * ignored
 *
 * Individual parameters can also be sequenced through the preset sequencer.
 * They must be registered with the PresetSequencer through registerParameter()
 * or the streaming (<<) operator, even if they are registered with the
 * PresetHandler.
 *
 * The line should start with the '+' character followed by the delta time to
 * the previous line. Note that parameters have delta times relative to both
 * preset and parameter steps, but preset events are relative to other preset
 * steps and ignore parameter and event steps.
 *
 * @code
 * preset1:0.0:3.0
 * +0.1:/X:0.3
 * +0.1:/X:0.4
 * +0.1:/X:0.5
 * preset2:4.0:2.0
 * ::
 * @endcode
 *
 * The directory where sequences are loaded is taken from the PresetHandler
 * object registered with the sequencer.
 *
 */
class PresetSequencer : public osc::MessageConsumer {
  friend class Composition;

public:
  PresetSequencer(
      TimeMasterMode timeMasterMode = TimeMasterMode::TIME_MASTER_CPU);
  ~PresetSequencer() override;

  typedef enum { PRESET, EVENT, PARAMETER } StepType;

  struct Step {
    StepType type = PRESET;
    std::string name;
    float morphTime; // The time to get to the preset
    float waitTime;  // The time to stay in the preset before the next step
                     //    float morphShape; // Shape of interpolation
    std::vector<VariantValue> params;
  };

  typedef struct {
    std::string eventName;
    std::function<void(void *data, std::vector<VariantValue> &params)> callback;
    void *callbackData;
  } EventCallback;

  /**
   * @brief Start playing the sequence specified
   * @param sequenceName
   * @param timeScale Times in sequence are multiplied by this factor
   *
   * There is a single sequencer engine in the PresetSequencer class, so if
   * a sequence is playing when this command is issued, the current playback
   * is interrupted and the new sequence requested starts immediately.
   */
  void playSequence(std::string sequenceName, double timeScale = 1.0f,
                    double timeOffset = 0.0);

  void stopSequence(bool triggerCallbacks = true);

  /**
   * @brief Set time into the current sequence.
   * @param time
   *
   * Will load the state at the time position for the currently playing se
   */
  void setTime(double time);

  /**
   * @brief Load sequence an prepare for playback
   */
  void rewind();

  bool playbackFinished() { return mSteps.size() == mCurrentStep; }

  /**
   * @brief getSequenceList returns a list of sequences in the current sequence
   * directory
   * @return a list of sequence names without path and without the '.sequence'
   * extension
   */
  std::vector<std::string> getSequenceList();

  /**
   * @brief setDirectory sets the working directory for the PresetSequencer
   * @param directory
   *
   * This function can be used in cases where the actual sequencer is not
   * needed, because preset recall and morphing is done by the PresetHandler.
   * This is useful for instance if you need to query the step of a sequence
   * for a different purpose. If a PresetHandler is registered, this value
   * is ignored.
   */
  void setDirectory(std::string directory) { mDirectory = directory; }

  /**
   * @brief Sets the sub-directory for the registered PresetHandler.
   *
   * Useful when you need to set the sub-dir but don't have easy access
   * to the PresetHandler object.
   */
  void setHandlerSubDirectory(std::string subDir);

  inline bool running() { return mRunning; }

  /**
   * @brief Register preset handler with sequencer
   * @param presetHandler
   * @return
   *  Sequencer will recall presets through the registered
   * preset handler. The sequencer's directory is set to
   * the preset handler's directory
   */
  PresetSequencer &registerPresetHandler(PresetHandler &presetHandler);

  /**
   * @brief Register PresetHandler through the << operator
   * @param presetHandler
   * @return reference to this object
   */
  PresetSequencer &operator<<(PresetHandler &presetHandler) {
    return registerPresetHandler(presetHandler);
  }

  PresetSequencer &operator<<(ParameterMeta &param) {
    return registerParameter(param);
  }

  PresetSequencer &registerParameter(ParameterMeta &param) {
    mParameters.push_back(&param);
    return *this;
  }

  /**
   * @brief Load steps from a sequence file
   * @param sequenceName The name of the sequence
   * @param timeScale The times in the sequence are multiplied by this factor
   * @return the steps
   *
   * The sequence is searched in the PresetHandler current path or the
   *  PresetSequencer's directory if PresetHandler not registered.
   */
  std::vector<Step> loadSequence(std::string sequenceName,
                                 double timeScale = 1.0);

  std::string currentSequence() { return mCurrentSequence; }

  /**
   * @brief registerEventCommand registers a function associated with an event
   * command
   *
   * Sequences can have events commands prefixed by @. Whenever an event is
   * found in a sequence, it triggers the callback registered here if the
   * event name matches
   */
  void registerEventCommand(
      std::string eventName,
      std::function<void(void *data, std::vector<VariantValue> &params)>
          callback,
      void *data);

  void setOSCSubPath(std::string subPath) { mOSCsubPath = subPath; }

  /**
   * @brief registerBeginCallback
   *
   * When registered this function is called from the playback thread as soon
   * as it is ready to start playing before calling the first step.
   */
  void registerBeginCallback(
      std::function<void(PresetSequencer *sender)> beginCallback);

  void enableBeginCallback(bool enable) { mBeginCallbackEnabled = enable; }

  void toggleEnableBeginCallback() {
    mBeginCallbackEnabled = !mBeginCallbackEnabled;
  }

  /**
   * @brief registerEndCallback
   *
   * The callback provides a finished argument. If the sequence reached the end
   * naturally, this argument will be passed as true. If it was stopped by the
   * user prematurely, it will send false.
   */
  void registerEndCallback(
      std::function<void(bool finished, PresetSequencer *sender)> endCallback);

  void enableEndCallback(bool enable) { mEndCallbackEnabled = enable; }
  void toggleEnableEndCallback() { mEndCallbackEnabled = !mEndCallbackEnabled; }

  void registerTimeChangeCallback(std::function<void(float)> func,
                                  float minTimeDeltaSec = -1.0);

  float getSequenceStartOffset(std::string sequenceName);
  float getSequenceTotalDuration(std::string sequenceName);

  // For programmatic control of the sequencer:
  void clearSteps();

  /**
   * @brief appendStep
   * @param newStep
   *
   * This function will block while the sequence is playing. Always stop
   * sequence before calling this function.
   */
  void appendStep(Step &newStep);

  void setTimeMaster(TimeMasterMode masterMode);

  /**
   * @brief step sequencer forward dt amount of time
   * @param dt amount of time (seconds) to step
   *
   * Any parameter and preset events that fall within this delta time will
   * be applied.
   */
  void stepSequencer(double dt);

  /**
   * @brief move sequencer forward by time set using setSequencerStepTime()
   */
  void stepSequencer();

  /**
   * @brief setSequencerStepTime
   * @param stepTime in seconds
   */
  void setSequencerStepTime(double stepTime) { mGranularity = stepTime * 1e9; }

  void setVerbose(bool newVerbose);

protected:
  virtual bool consumeMessage(osc::Message &m,
                              std::string rootOSCPath) override;

  void processTimeChangeRequest();
  void updateTime(double time);

  void updateSequencer();

private:
  static void sequencerFunction(PresetSequencer *sequencer);

  std::string buildFullPath(std::string sequenceName);

  void startCpuThread();
  void stopCpuThread();

  std::vector<Step> mSteps;
  std::string mDirectory;
  PresetHandler *mPresetHandler{nullptr};
  std::vector<ParameterMeta *> mParameters;
  std::string mOSCsubPath;
  std::string mCurrentSequence;
  bool mVerbose{false};

  std::atomic<float> mTimeRequest{-1.0f}; // Request setting the current time.
                                          // Passes info to playback thread

  TimeMasterMode mTimeMasterMode;

  bool mSequencerActive{false};
  bool mRunning;
  bool mStartRunning;
  std::queue<Step> mParameterList;
  double mCurrentTime = 0.0; // Current time (in seconds)
  double mTargetTime;
  double mLastPresetTime; // To anchor parameter deltas
  double mParameterTargetTime{0};
  double mLastTimeUpdate = 0.0;

  uint64_t mGranularity = 10e6; // nanoseconds time for default step size
  bool mBeginCallbackEnabled;
  std::function<void(PresetSequencer *)> mBeginCallback;
  bool mEndCallbackEnabled;
  std::function<void(bool, PresetSequencer *)> mEndCallback;
  std::vector<EventCallback> mEventCallbacks;
  std::vector<std::function<void(float)>> mTimeChangeCallbacks;
  float mTimeChangeMinTimeDelta = 0.05f;

  // CPU thread
  std::unique_ptr<std::thread> mSequencerThread;
  std::mutex mSequenceLock;
  uint64_t mCurrentStep;
  PresetHandler::ParameterStates mStartValues;
  std::mutex mPlayWaitLock;
  std::condition_variable mPlayWaitVariable;
  std::shared_ptr<std::promise<void>> mPlayPromiseObj;
};

} // namespace al

#endif // AL_PRESETSEQUENCER_H
