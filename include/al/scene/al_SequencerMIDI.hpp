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
 * @brief The SequencerMIDI class connects PolySynth objects to MIDI messages
 * @ingroup Scene
 *
 *
@code
PolySynth synth;

SequencerMIDI seq;
seq.open(0, synth);

seq.connectNoteOnToFunction(
          [&](int note, int vel, int chan) {
    if (chan != 1) {
        return;
    }
    auto voicePtr = mSynth->getVoice<Voice1>();

    if (voicePtr) {
        // Set voice params from MIDI data
        voice->note = note;
        voice->vel = vel;
        // then trigger note
        mScene.triggerOn(voice, 0);
    }
});
@endcode
 *
 */
class SequencerMIDI : public MIDIMessageHandler {
public:
  SequencerMIDI() {}

  SequencerMIDI(int deviceIndex);

  SequencerMIDI(int deviceIndex, PolySynth &synth);

  void open(int deviceIndex, PolySynth &synth);

  void open(int deviceIndex);

  //  [[deprecated("Use open()")]] void init(int deviceIndex, PolySynth &synth);

  void close();

  bool isOpen();

  void setSynthSequencer(PolySynth &synth) { mSynth = &synth; }

  void connectNoteOnToFunction(std::function<void(int, int, int)> function);
  void connectNoteOffToFunction(std::function<void(int, int, int)> function);

  virtual void onMIDIMessage(const MIDIMessage &m) override;

private:
  PolySynth *mSynth;

  RtMidiIn mRtMidiIn;
  std::vector<std::function<void(int, int, int)>> mNoteOnFunctions;
  std::vector<std::function<void(int, int, int)>> mNoteOffFunctions;
};

} // namespace al

#endif // AL_SEQUENCERMIDI_H
