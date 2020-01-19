#ifndef AL_SEQUENCERECORDER_H
#define AL_SEQUENCERECORDER_H

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
        Preset sequence recorder

        File author(s):
        Andrés Cabrera mantaraya36@gmail.com
*/

#include <condition_variable>
#include <cstring>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>

#include "al/ui/al_PresetHandler.hpp"
#include "al/ui/al_PresetSequencer.hpp"

namespace al {

/**
 * @brief The SequenceRecorder class records preset changes in a ".sequence"
 * file
 * @ingroup UI
 *
 * The sequences recorded can be played back using the PresetSequencer class.
 *
 * @code
 * Parameter polyX("polyX", "", 0.0, "", -99, 99);
 * Parameter polyY("polyY", "", 0.0, "", -99, 99);
 * Parameter polyZ("polyZ", "", 0.0, "", -99, 99);
 * PresetHandler presets("hydrogen_presets");
 * SequenceRecorder recorder;
 *
 * presets << polyX << polyY <<polyZ;
 * recorder << presets;
 *
 * recorder.startRecord("seqName");
 * ...
 * recorder.stopRecord();
 *
 * @endcode
 */
class SequenceRecorder : public osc::MessageConsumer {
 public:
  SequenceRecorder();

  ~SequenceRecorder() override {}

  /**
   * @brief startRecord begins recording preset changes
   * @param name default name is "new_seq"
   * @param overwrite whether to force overwrite
   *
   * By default startRecord() will not overwrite, and will append a number to
   * the sequence name specified.
   */
  void startRecord(std::string name = "", bool overwrite = false);
  void stopRecord();

  bool recording() { return mRecording; }

  void setMaxRecordTime(al_sec maxTime) { mMaxRecordTime = maxTime; }

  std::string lastSequenceName();
  std::string lastSequenceSubDir();

  /**
   * @brief setDirectory sets the working directory for the SequenceRecorder
   * @param directory
   *
   * If a PresetHandler is registered, this value
   * is ignored.
   */
  void setDirectory(std::string directory);

  std::string getCurrentPath() { return mPresetHandler->getCurrentPath(); }

  SequenceRecorder &operator<<(PresetHandler &handler) {
    registerPresetHandler(handler);
    return *this;
  }

  void registerPresetHandler(PresetHandler &handler) {
    mPresetHandler = &handler;
    if (mPresetHandler) {
      for (auto *presetParam : mPresetHandler->parameters()) {
        for (auto *p : mParameters) {
          if (presetParam->getFullAddress() == p->getFullAddress()) {
            std::cerr << "ERROR: Parameters registered with sequence recorder "
                         "must not be part of registered presets: "
                      << p->getFullAddress() << std::endl;
          }
        }
      }
      mPresetHandler->registerPresetCallback(SequenceRecorder::presetChanged,
                                             (void *)this);
    }
  }

  SequenceRecorder &operator<<(ParameterMeta &param) {
    registerParameter(param);
    return *this;
  }

  void registerParameter(ParameterMeta &p);

 protected:
  virtual bool consumeMessage(osc::Message &m,
                              std::string rootOSCPath) override;

 private:
  static void presetChanged(int index, void *sender, void *userData);
  static void recorderFunction(SequenceRecorder *recorder,
                               std::string sequenceName);

  //	struct Step {
  //		std::string presetName;
  //		float delta; // The time to get to the preset
  //		float duration; // The time to stay in the preset before the
  // next step
  //	};

  std::string mDirectory;
  PresetHandler *mPresetHandler;
  std::vector<ParameterMeta *> mParameters;
  //	std::string mPresetName;
  bool mOverwrite;
  std::string mLastSequenceName;
  std::string mLastSequenceSubDir;

  std::mutex mSequenceLock;
  std::condition_variable mSequenceConditionVar;

  bool mRecording;
  std::thread *mRecorderThread;

  PresetSequencer::Step mStepToInsert;

  al_sec mMaxRecordTime;
};

}  // namespace al

#endif  // AL_SEQUENCERECORDER_H
