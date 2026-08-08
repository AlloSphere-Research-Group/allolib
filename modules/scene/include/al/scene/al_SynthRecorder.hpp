#ifndef AL_SYNTHRECORDER_HPP
#define AL_SYNTHRECORDER_HPP

/*	Allocore --
        Multimedia / virtual environment application class library

        Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012-2018. The Regents of the University of California.
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
        SynthVoice sequence recorder

        File author(s):
        Andrés Cabrera mantaraya36@gmail.com
*/

#include <chrono>
#include <fstream>

#include "al/io/al_File.hpp"
#include "al/scene/al_SynthSequencer.hpp"

namespace al {

/**
 * @brief The SynthRecorder class records the events arriving at a PolySynth
 *
 * @ingroup Scene
 *
 * Events are stored in a text file that looks like:
 *
 * @code
 * + 0 72 SineEnv 6.5 523.251 0.5 0.2 1 0
 * + 0.149975 74 SineEnv 6.5 587.33 0.5 0.2 1 0
 * - 0.183213 72
 * + 0.283204 76 SineEnv 6.5 659.255 0.5 0.2 1 0
 * - 0.366428 74
 * - 0.483025 76
 * @endcode
 *
 * The '+' command triggers the start of an event and the '-' command triggers
 * the end of the event. The first number after the command is the time at
 * which it should happen and the second one is the id of the event, used to
 * connect the 'trigger off' to a previous 'trigger on'.
 *
 * Alternatively, the sequence can be recorded in CPP_FORMAT that produces C++
 * code that can be pasted to deliver the sequence.
 *
 * The sequences stored in the text file can be played back using SynthSequencer
 * You must make sre that the synthesizers referenced in the sequence have
 * enough allocated polyphony for the whole sequence or alternatively that
 * the class has been registered to the PolySynth using registerSynthClass().
 *
 * @code
 * SynthSequencer seq;
 * // Pre allocate voices
 * seq.synth().allocatePolyphony<SineEnv>(64);
 * // Register class with PolySynth
 * seq.synth().registerSynthClass<SineEnv>("SineEnv");
 * @endcode
 *
 */
class SynthRecorder {
 public:
  typedef enum {
    SEQUENCER_EVENT,     // Events have duration (uses '@' command only)
    SEQUENCER_TRIGGERS,  // Store events as they were received trigger on and
                         // trigger off can be separate entries (uses '+' and
                         // '-' text commands)
    CPP_FORMAT,          // Saves code that can be copy-pasted into C++
    NONE
  } TextFormat;

  SynthRecorder(TextFormat format = SEQUENCER_EVENT) { mFormat = format; }

  void setDirectory(std::string path) {
    if (!File::exists(path)) {
      if (!Dir::make(path)) {
        std::cout << "Error creating directory: " << path << std::endl;
      }
    }
    mDirectory = path;
  }

  void startRecord(std::string name = "", bool overwrite = false,
                   bool startOnEvent = true);

  void stopRecord();

  void setMaxRecordTime(al_sec maxTime) { mMaxRecordTime = maxTime; }

  void verbose(bool verbose) { mVerbose = true; }
  bool verbose() { return mVerbose; }

  //	std::string lastSequenceName();
  //	std::string lastSequenceSubDir();

  //	std::string getCurrentPath() { return mPresetHandler->getCurrentPath();
  //}

  SynthRecorder &operator<<(PolySynth &handler) {
    registerPolySynth(handler);
    return *this;
  }

  void registerPolySynth(PolySynth &polySynth);

  /**
   * @brief onTriggerOn callback for trigger on events
   * @param voice
   * @param offsetFrames
   * @param id
   * @param userData
   */
  static bool onTriggerOn(SynthVoice *voice, int offsetFrames, int id,
                          void *userData) {
    std::chrono::high_resolution_clock::time_point now =
        std::chrono::high_resolution_clock::now();
    SynthRecorder *rec = static_cast<SynthRecorder *>(userData);
    if (rec->mRecording) {
      std::unique_lock<std::mutex> lk(rec->mSequenceLock);
      if (rec->mStartOnEvent) {
        rec->mSequenceStart = now;
        rec->mStartOnEvent = false;
      }
      std::chrono::duration<double> diff = now - rec->mSequenceStart;
      std::vector<VariantValue> pFields = voice->getTriggerParams();

      SynthEvent event;
      event.type = SynthEventType::TRIGGER_ON;
      event.id = voice->id();
      event.time = diff.count();
      event.synthName = demangle(typeid(*voice).name());
      event.duration = -1;
      event.pFields = pFields;
      rec->mSequence.push_back(event);
      if (rec->verbose()) {
        std::cout << "trigger at " << event.time << ":" << event.synthName
                  << ":" << voice->id() << std::endl;
      }
    }
    return true;
  }

  /**
   * @brief onTriggerOff callback for trigger off events
   * @param id
   * @param userData
   */
  static bool onTriggerOff(int id, void *userData) {
    std::chrono::high_resolution_clock::time_point now =
        std::chrono::high_resolution_clock::now();
    SynthRecorder *rec = static_cast<SynthRecorder *>(userData);
    if (rec->mRecording) {
      std::unique_lock<std::mutex> lk(rec->mSequenceLock);
      //            if (rec->mStartOnEvent) {
      //                rec->mSequenceStart = now;
      //                rec->mStartOnEvent = false;
      //            }
      std::chrono::duration<double> diff = now - rec->mSequenceStart;

      SynthEvent event;
      event.type = SynthEventType::TRIGGER_OFF;
      event.id = id;
      event.time = diff.count();
      //            event.synthName = demangle(typeid (*voice).name() );
      event.duration = -1;
      //            for (int i = 0; i < numFields; i++) {
      //                event.pFields.push_back(pFields[i]);
      //            }
      rec->mSequence.push_back(event);
      if (rec->verbose()) {
        std::cout << "trigger OFF at " << event.time << ":" << event.synthName
                  << ":" << id << std::endl;
      }
    }
    return true;
  }

 private:
  std::string mDirectory;
  PolySynth *mPolySynth{nullptr};
  TextFormat mFormat;
  bool mVerbose;

  bool mOverwrite;
  std::string mSequenceName;

  std::mutex mSequenceLock;

  bool mRecording{false};
  bool mStartOnEvent{true};

  al_sec mMaxRecordTime;
  std::chrono::high_resolution_clock::time_point mSequenceStart;
  std::vector<SynthEvent> mSequence;
};

// Implementation

}  // namespace al

#endif  // AL_SYNTHSEQUENCER_HPP
