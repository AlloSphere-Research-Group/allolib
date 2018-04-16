#ifndef AL_SYNTHGUI_HPP
#define AL_SYNTHGUI_HPP

/*	Allocore --
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
	SynthGUI class for simple GUI for Synth classes

	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <string>
#include <map>

#include "al/util/ui/al_ParameterMIDI.hpp"
#include "al/util/ui/al_SynthSequencer.hpp"
#include "al/util/ui/al_SynthRecorder.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/util/ui/al_Preset.hpp"
#include "al/util/imgui/al_Imgui.hpp"


namespace al
{
/**
 * @brief The SynthGUI class
 *
 * You must call init() before any draw calls.
 */
class SynthGUI {
public:

    SynthGUI &registerParameter(Parameter &param);

    SynthGUI &registerPresetHandler(PresetHandler &presetHandler);

    /// Register parameter using the streaming operator. A widget is shown to control it.
    SynthGUI &operator << (Parameter& newParam){ return registerParameter(newParam); }

    /// Register parameter using the streaming operator. A widget is shown to control it.
    SynthGUI &operator << (Parameter* newParam){ return registerParameter(*newParam); }

    /// Register preset handler using the streaming operator. GUI widgets for preset control are shown.
    SynthGUI &operator << (PresetHandler& ph){ return registerPresetHandler(ph); }

    /// Register preset handler using the streaming operator. GUI widgets for preset control are shown.
    SynthGUI &operator << (PresetHandler* ph){ return registerPresetHandler(*ph); }

    /// Register a SynthRecorder. This will display GUI widgets to control it
    SynthGUI & operator<< (SynthRecorder &recorder) { registerSynthRecorder(recorder);  return *this; }

    void registerSynthRecorder(SynthRecorder &recorder);

    /// Register a SynthSequencer. This will display GUI widgets to control it
    /// Will also register the PolySynth contained within it.
    SynthGUI & operator<< (SynthSequencer &seq) { registerSynthSequencer(seq);  return *this; }

    void registerSynthSequencer(SynthSequencer &seq);

    void onDraw(Graphics &g);

    /**
     * @brief Call to set if this GUI manages ImGUI
     *
     * Set to false if you want to have additional ImGUI windows.
     * You will have to handle the initialization, cleanup and
     * begin/end calls yourself.
     */
    void manageImGUI(bool manage) {mManageIMGUI = manage;}

    void init() {
        if (mManageIMGUI) {
            initIMGUI();
        }
    }

    /**
     * @brief Call begin() at the start of the outer draw call, if not managing ImGUI
     */
    void begin() {
        if (mManageIMGUI) {
            beginIMGUI();
        }
    }

    /**
     * @brief Call begin() at the end of the outer draw call, if not managing ImGUI
     */
    void end() {
        if (mManageIMGUI) {
            endIMGUI();
        }
    }

    void cleanup() {
        if (mManageIMGUI) {
            shutdownIMGUI();
        }
    }


protected:

private:
    std::map<std::string, std::vector<Parameter *>> mParameters;
    PresetHandler *mPresetHandler {nullptr};
    SynthRecorder *mSynthRecorder {nullptr};
    SynthSequencer *mSynthSequencer {nullptr};
    PolySynth *mPolySynth {nullptr};

    bool mStoreButtonOn {false};
    bool mRecordButtonValue {false};
    bool mOverwriteButtonValue {true};
    bool mManageIMGUI {true};
};

}

#endif
