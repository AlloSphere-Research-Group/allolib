#ifndef AL_ControlGUI_HPP
#define AL_ControlGUI_HPP

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
    ControlGUI class for simple GUI for Synth classes

	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <string>
#include <map>

#include "al/core/io/al_ControlNav.hpp"
#include "al/util/ui/al_ParameterMIDI.hpp"
#include "al/util/ui/al_SynthSequencer.hpp"
#include "al/util/ui/al_SynthRecorder.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/util/ui/al_Preset.hpp"
#include "al/util/imgui/al_Imgui.hpp"


namespace al
{
/**
 * @brief The ControlGUI class
 *
 * You must call init() before any draw calls.
 */
class ControlGUI {
public:
    ControlGUI &registerParameter(Parameter &param);

    /// Register parameter using the streaming operator. A widget is shown to control it.
    ControlGUI &operator << (Parameter& newParam){ return registerParameter(newParam); }

    /// Register parameter using the streaming operator. A widget is shown to control it.
    ControlGUI &operator << (Parameter* newParam){ return registerParameter(*newParam); }


    ControlGUI &registerParameterBool(ParameterBool &param);

    /// Register parameter using the streaming operator. A widget is shown to control it.
    ControlGUI &operator << (ParameterBool& newParam){ return registerParameter(newParam); }

    /// Register parameter using the streaming operator. A widget is shown to control it.
    ControlGUI &operator << (ParameterBool* newParam){ return registerParameter(*newParam); }


    ControlGUI &registerNav(Nav &nav);

    /// Register nav using the streaming operator. A set of widgets are shown to control it.
    ControlGUI &operator << (Nav &nav){ return registerNav(nav); }


    ControlGUI &registerPresetHandler(PresetHandler &presetHandler);

    /// Register preset handler using the streaming operator. GUI widgets for preset control are shown.
    ControlGUI &operator << (PresetHandler& ph){ return registerPresetHandler(ph); }

    /// Register preset handler using the streaming operator. GUI widgets for preset control are shown.
    ControlGUI &operator << (PresetHandler* ph){ return registerPresetHandler(*ph); }


    void registerSynthRecorder(SynthRecorder &recorder);

    /// Register a SynthRecorder. This will display GUI widgets to control it
    ControlGUI & operator<< (SynthRecorder &recorder) { registerSynthRecorder(recorder);  return *this; }


    void registerSynthSequencer(SynthSequencer &seq);

    /// Register a SynthSequencer. This will display GUI widgets to control it
    /// Will also register the PolySynth contained within it.
    ControlGUI & operator<< (SynthSequencer &seq) { registerSynthSequencer(seq);  return *this; }

    /**
     * @brief draws the GUI
     */
    void draw(Graphics &g);

    /**
     * @brief Call to set if this GUI manages ImGUI
     *
     * Set to false if you want to have additional ImGUI windows.
     * You will have to handle the initialization, cleanup and
     * begin/end calls yourself.
     */
    void manageImGUI(bool manage) {mManageIMGUI = manage;}

    /**
     * @brief initialize ImGUI.
     * @param x x position for the control window
     * @param y y position for the control window
     *
     * This function must be called before anu call to begin() or draw()
     */
    void init(int x = 5, int y = 5);

    /**
     * @brief Call begin() at the start of the outer draw call, if not managing ImGUI
     */
    void begin();

    /**
     * @brief Call begin() at the end of the outer draw call, if not managing ImGUI
     */
    void end();

    void cleanup();

    /**
     * @brief usingInput returns true if the mouse is within the imgui window
     *
     * Can be used to selectively turn off navigation when using ImGUI you
     * should place this within your onAnimate() callback:
     *
     * @code
     *  virtual void onAnimate(double dt) override {
     *      navControl().active(!gui.usingInput());
     *  }
     * @endcode
     *
     * Note that if the call is placed outside the ImGUI begin and end calls,
     * then the data is likely to be one frame late. This is often not a big issue
     * and simplifies the code.
     */
    bool usingInput() {return imgui_is_using_input();}


protected:

private:
    std::map<std::string, std::vector<Parameter *>> mParameters;
    std::map<std::string, std::vector<ParameterVec3 *>> mParameterVec3s;
    std::map<std::string, std::vector<ParameterVec4 *>> mParameterVec4s;

    PresetHandler *mPresetHandler {nullptr};
    SynthRecorder *mSynthRecorder {nullptr};
    SynthSequencer *mSynthSequencer {nullptr};
    PolySynth *mPolySynth {nullptr};
    Nav *mNav {nullptr};

    int mX, mY;
    int mId;

    bool mStoreButtonOn {false};
    bool mRecordButtonValue {false};
    bool mOverwriteButtonValue {true};
    bool mManageIMGUI {true};

    int mCurrentSequencerItem {0};
    char** mSequencerItems;

};

}

#endif
