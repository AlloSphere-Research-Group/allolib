#ifndef AL_PRESETMIDI_H
#define AL_PRESETMIDI_H

/*	Allocore --
        Multimedia / virtual environment application class library

        Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012-2015. The Regents of the University of California.
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
        Class to connect MIDI Input to PresetHandler objects
        File author(s):
        Andrés Cabrera mantaraya36@gmail.com
*/

#include "al/io/al_MIDI.hpp"
#include "al/ui/al_PresetHandler.hpp"

namespace al {

/**
 * @brief The PresetMIDI class connects PresetHandler objects to MIDI messages
 * @ingroup UI
 *
@code

@endcode
 *
 */
class PresetMIDI : public MIDIMessageHandler {
 public:
  PresetMIDI() {}

  PresetMIDI(int deviceIndex) : mPresetHandler(nullptr) {
    MIDIMessageHandler::bindTo(mRtMidiIn);
    try {
      mRtMidiIn.openPort(deviceIndex);
      printf("PresetMIDI: Opened port to %s\n",
             mRtMidiIn.getPortName(deviceIndex).c_str());
    } catch (RtMidiError &error) {
      std::cerr << error.getMessage() << std::endl;
      std::cerr << "PresetMIDI Warning: Could not open MIDI port "
                << deviceIndex << std::endl;
    }
    mMorphBinding.channel = mMorphBinding.controlNumber = -1;
  }

  PresetMIDI(int deviceIndex, PresetHandler &presetHandler)
      : PresetMIDI(deviceIndex) {
    mPresetHandler = &presetHandler;
    setPresetHandler(*mPresetHandler);
  }

  void enable() { mEnabled = true; }
  void disable() { mEnabled = false; }

  void init(int deviceIndex, PresetHandler &presetHandler) {
    open(deviceIndex, presetHandler);
  }

  void open(int deviceIndex, PresetHandler &presetHandler) {
    open(deviceIndex);
    setPresetHandler(presetHandler);
  }

  void open(int deviceIndex) {
    MIDIMessageHandler::bindTo(mRtMidiIn);

    if (mRtMidiIn.isPortOpen()) {
      mRtMidiIn.closePort();
    }
    try {
      if (deviceIndex >= 0 && deviceIndex < (int)mRtMidiIn.getPortCount()) {
        mRtMidiIn.openPort(deviceIndex);
        printf("PresetMIDI: Opened port to %s\n",
               mRtMidiIn.getPortName(deviceIndex).c_str());
      } else {
        std::cerr << "PresetMIDI Warning: Could not open MIDI port "
                  << deviceIndex << std::endl;
      }
    } catch (RtMidiError &error) {
      std::cerr << error.getMessage() << std::endl;
      std::cerr << "PresetMIDI Warning: Could not open MIDI port "
                << deviceIndex << std::endl;
    }
  }

  void close() {
    mRtMidiIn.closePort();
    MIDIMessageHandler::clearBindings();
  }

  bool isOpen() { return mRtMidiIn.isPortOpen(); }

  void setPresetHandler(PresetHandler &presetHandler) {
    mPresetHandler = &presetHandler;
  }

  struct NoteBinding {
    int noteNumber;
    int channel;
    int presetIndex;
  };

  struct ProgramBinding {
    int programNumber;
    int channel;
    int presetIndex;
  };

  struct MorphBinding {
    int controlNumber;
    int channel;
    float min;
    float max;
  };

  void connectNoteToPreset(int channel, float presetLow, int noteLow,
                           float presetHigh = -1, int noteHigh = -1);

  void connectProgramToPreset(int channel, float presetLow, int programLow,
                              float presetHigh = -1, int programHigh = -1);

  void setMorphControl(int controlNumber, int channel, float min, float max);

  virtual void onMIDIMessage(const MIDIMessage &m) override;

  std::vector<NoteBinding> getCurrentNoteBindings() { return mNoteBindings; }
  std::vector<ProgramBinding> getCurrentProgramBindings() {
    return mProgramBindings;
  }

 private:
  bool mEnabled{true};
  MorphBinding mMorphBinding;

  PresetHandler *mPresetHandler;

  RtMidiIn mRtMidiIn;
  std::vector<NoteBinding> mNoteBindings;
  std::vector<ProgramBinding> mProgramBindings;
};

}  // namespace al

#endif  // AL_PRESETMIDI_H
