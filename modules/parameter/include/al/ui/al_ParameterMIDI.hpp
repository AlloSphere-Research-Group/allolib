#ifndef AL_PARAMETERMIDI_H
#define AL_PARAMETERMIDI_H

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
        Class to connect MIDI Input to Parameter objects
        File author(s):
        Andrés Cabrera mantaraya36@gmail.com
*/

#include "al/io/al_MIDI.hpp"
#include "al/ui/al_Parameter.hpp"

#undef min
#undef max
namespace al {

/**
 * @brief The ParameterMIDI class connects Parameter objects to MIDI messages
 * @ingroup UI
 *
 *
@code
    Parameter Size("Size", "", 1.0, "", 0, 1.0);
    Parameter Speed("Speed", "", 0.05, "", 0.01, 0.3);

    ParameterMIDI parameterMIDI;

    parameterMIDI.connectControl(Size, 1, 1);
    parameterMIDI.connectControl(Speed, 10, 1);

    parameterMIDI.init(0); // Open MIDI device
@endcode
 *
 */
class ParameterMIDI : public MIDIMessageHandler {
 public:
  ParameterMIDI() {}

  ParameterMIDI(unsigned int deviceIndex, bool verbose = false);

  void open(unsigned int deviceIndex = 0);

  void open(int deviceIndex, bool verbose);

  [[deprecated("Use open()")]] void init(int deviceIndex = 0,
                                         bool verbose = false);

  void close();

  void connectControl(Parameter &param, int controlNumber, int channel) {
    connectControl(param, controlNumber, channel, param.min(), param.max());
  }

  void connectControl(Parameter &param, int controlNumber, int channel,
                      float min, float max);

  /**
   * Connect multiple MIDI controls to multivalue parameter.
   *
   * This allows connecting 3 different MIDI controls to the R,G,B values
   * of ParameterColor or to the x,y,z values of ParameterPose
   */

  void connectControls(ParameterMeta &param, std::vector<int> controlNumbers,
                       int channel = 1, std::vector<float> min = {},
                       std::vector<float> max = {});

  /**
   * @brief connectNoteToValue
   * @param param the parameter to bind
   * @param channel MIDI channel (1-16)
   * @param min The parameter value to map the lowest note
   * @param low The MIDI note number for the lowest (or only) note to map
   * @param max The value unto which the highest note is mapped
   * @param high The highest MIDI note number to map
   */
  void connectNoteToValue(Parameter &param, int channel, float min, int low,
                          float max = -1, int high = -1);

  void connectNoteToToggle(ParameterBool &param, int channel, int note);

  void connectNoteToIncrement(Parameter &param, int channel, int note,
                              float increment);

  bool isOpen() { return mRtMidiIn.isPortOpen(); }

  virtual void onMIDIMessage(const MIDIMessage &m) override;

  void verbose(bool v) { mVerbose = v; }

  struct ControlBinding {
    int controlNumber;
    int channel;
    Parameter *param;
    float min, max;
  };

  struct AbstractBinding {
    std::vector<int> controlNumbers;
    int channel;
    ParameterMeta *param;
    std::vector<float> min;
    std::vector<float> max;
  };

  struct NoteBinding {
    int noteNumber;
    int channel;
    float value;
    Parameter *param;
  };

  struct ToggleBinding {
    int noteNumber;
    int channel;
    bool toggle;
    ParameterBool *param;
  };

  struct IncrementBinding {
    int noteNumber;
    int channel;
    float increment;
    Parameter *param;
  };

  std::vector<ControlBinding> getCurrentControlBindings() {
    return mControlBindings;
  }
  std::vector<NoteBinding> getCurrentNoteBindings() { return mNoteBindings; }

 private:
  RtMidiIn mRtMidiIn;
  bool mVerbose;
  std::vector<ControlBinding> mControlBindings;
  std::vector<NoteBinding> mNoteBindings;
  std::vector<ToggleBinding> mToggleBindings;
  std::vector<IncrementBinding> mIncrementBindings;
  std::vector<AbstractBinding> mAbstractBindings;
};

}  // namespace al

#endif  // AL_PARAMETERMIDI_H
