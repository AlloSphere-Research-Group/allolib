#ifndef AL_SEQUENCERMIDI_H
#define AL_SEQUENCERMIDI_H

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
        Class to connect MIDI Input to SynthSequencer objects
        File author(s):
        Andrés Cabrera mantaraya36@gmail.com
*/

#include "al/io/al_MIDI.hpp"
#include "al/scene/al_SynthSequencer.hpp"

namespace al {

/**
 * @brief The PresetMIDI class connects PresetHandler objects to MIDI messages
 * @ingroup Scene
 *
 *
@code

@endcode
 *
 */
class SequencerMIDI : public MIDIMessageHandler {
 public:
  SequencerMIDI() {}

  SequencerMIDI(int deviceIndex) : mSynth(nullptr) {
    MIDIMessageHandler::bindTo(mMidiIn);
    try {
      mMidiIn.openPort(deviceIndex);
      printf("PresetMIDI: Opened port to %s\n",
             mMidiIn.getPortName(deviceIndex).c_str());
    } catch (al::MIDIError error) {
      std::cout << "PresetMIDI Warning: Could not open MIDI port "
                << deviceIndex << std::endl;
    }
  }

  SequencerMIDI(int deviceIndex, PolySynth &synth)
      : SequencerMIDI(deviceIndex) {
    setSynthSequencer(synth);
  }

  void init(int deviceIndex, PolySynth &synth) { open(deviceIndex, synth); }

  void open(int deviceIndex, PolySynth &synth) {
    open(deviceIndex);
    setSynthSequencer(synth);
  }

  void open(int deviceIndex) {
    MIDIMessageHandler::bindTo(mMidiIn);

    if (mMidiIn.isPortOpen()) {
      mMidiIn.closePort();
    }
    try {
      if (deviceIndex >= 0 && deviceIndex < (int)mMidiIn.getPortCount()) {
        mMidiIn.openPort(deviceIndex);
        printf("PresetMIDI: Opened port to %s\n",
               mMidiIn.getPortName(deviceIndex).c_str());
      } else {
        std::cerr << "PresetMIDI Warning: Could not open MIDI port "
                  << deviceIndex << std::endl;
      }
    } catch (al::MIDIError error) {
      std::cerr << "PresetMIDI Warning: Could not open MIDI port "
                << deviceIndex << std::endl;
    }
  }

  void close() {
    mMidiIn.closePort();
    MIDIMessageHandler::clearBindings();
  }

  bool isOpen() { return mMidiIn.isPortOpen(); }

  void setSynthSequencer(PolySynth &synth) { mSynth = &synth; }

  void connectNoteOnToFunction(std::function<void(int, int, int)> function) {
    mNoteOnFunctions.push_back(function);
  }
  void connectNoteOffToFunction(std::function<void(int, int, int)> function) {
    mNoteOffFunctions.push_back(function);
  }

  virtual void onMIDIMessage(const MIDIMessage &m) override {
    if (m.type() == MIDIByte::NOTE_ON && m.velocity() > 0) {
      for (auto function : mNoteOnFunctions) {
        //				std::cout << binding.channel << " " <<
        //binding.noteNumber << " " << binding.presetIndex << std::endl;
        //				std::cout << (int) m.channel() <<
        //std::endl;
        function(m.noteNumber(), m.velocity(), m.channel());
      }
    } else if (m.type() == MIDIByte::NOTE_OFF ||
               (m.type() == MIDIByte::NOTE_ON && m.velocity() == 0)) {
      for (auto function : mNoteOffFunctions) {
        //				std::cout << binding.channel << " " <<
        //binding.noteNumber << " " << binding.presetIndex << std::endl;
        //				std::cout << (int) m.channel() <<
        //std::endl;
        function(m.noteNumber(), m.velocity(), m.channel());
      }
    } else if (m.type() == MIDIByte::PROGRAM_CHANGE && m.velocity() > 0) {
      //            for(ProgramBinding binding: mProgramBindings) {
      //                //				std::cout <<
      //                binding.channel << " " << binding.noteNumber << " " <<
      //                binding.presetIndex << std::endl;
      //                //				std::cout << (int)
      //                m.channel() << std::endl; if (m.channel() ==
      //                binding.channel
      //                        && m.noteNumber() == binding.programNumber) {
      //                    //					m.print();
      //                    mPresetHandler->recallPreset(binding.presetIndex);
      //                }
      //            }
    } else if (m.type() == MIDIByte::CONTROL_CHANGE) {
      //            if (m.controlNumber() == mMorphBinding.controlNumber
      //                    && m.channel() == mMorphBinding.channel) {
      //                mPresetHandler->setMorphTime(mMorphBinding.min +
      //                m.controlValue() * (mMorphBinding.max -
      //                mMorphBinding.min));
      //            }
    }
  }

 private:
  PolySynth *mSynth;

  MIDIIn mMidiIn;
  std::vector<std::function<void(int, int, int)>> mNoteOnFunctions;
  std::vector<std::function<void(int, int, int)>> mNoteOffFunctions;
};

}  // namespace al

#endif  // AL_SEQUENCERMIDI_H
