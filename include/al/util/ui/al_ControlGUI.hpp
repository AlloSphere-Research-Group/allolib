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
#include "al/util/ui/al_Parameter.hpp"
#include "al/util/ui/al_ParameterBundle.hpp"
#include "al/util/ui/al_Preset.hpp"
#include "al/util/ui/al_PresetSequencer.hpp"
#include "al/util/scene/al_SynthSequencer.hpp"
#include "al/util/scene/al_SynthRecorder.hpp"
#include "al/util/scene/al_DynamicScene.hpp"
#include "al/util/imgui/al_Imgui.hpp"


namespace al
{

class GUIMarker
{
public:
    enum class MarkerType {
        GROUP_BEGIN,
        GROUP_END,
        SEPARATOR
    };
    GUIMarker(MarkerType type, const char *name = nullptr) {mMarkerType = type; mName = name;}

    MarkerType getType() {return mMarkerType;}
    const char *getName() {return mName;}
private:
    MarkerType mMarkerType;
    const char *mName;
};

/**
 * @brief The ControlGUI class
 *
 * You must call init() before any draw calls.
 */
class ControlGUI {
public:
    ControlGUI &registerParameterMeta(ParameterMeta &param);

    /// Register parameter using the streaming operator.
    ControlGUI &operator << (ParameterMeta& newParam){ return registerParameterMeta(newParam); }

    /// Register parameter using the streaming operator.
    ControlGUI &operator << (ParameterMeta* newParam){ return registerParameterMeta(*newParam); }

    ControlGUI &registerParameterBundle(ParameterBundle &bundle);

    /// Register parameter using the streaming operator.
    ControlGUI &operator << (ParameterBundle& newBundle){ return registerParameterBundle(newBundle); }

    /// Register parameter using the streaming operator.
    ControlGUI &operator << (ParameterBundle* newBundle){ return registerParameterBundle(*newBundle); }

    ControlGUI &registerNav(Nav &nav);

    /// Register nav using the streaming operator. A set of widgets are shown to control it.
    ControlGUI &operator << (Nav &nav){ return registerNav(nav); }

    ControlGUI &registerPresetHandler(PresetHandler &presetHandler, int numRows = -1, int numColumns = -1);

    /// Register preset handler using the streaming operator. GUI widgets for preset control are shown.
    ControlGUI &operator << (PresetHandler& ph){ return registerPresetHandler(ph); }

    /// Register preset handler using the streaming operator. GUI widgets for preset control are shown.
    ControlGUI &operator << (PresetHandler* ph){ return registerPresetHandler(*ph); }

    ControlGUI &registerPresetSequencer(PresetSequencer &presetSequencer);

    /// Register preset sequencer using the streaming operator. GUI widgets for preset sequencing are shown.
    ControlGUI &operator << (PresetSequencer& ps){ return registerPresetSequencer(ps); }


    ControlGUI &registerSequenceRecorder(SequenceRecorder &recorder);

    /// Register preset sequencer using the streaming operator. GUI widgets for preset sequencing are shown.
    ControlGUI &operator << (SequenceRecorder& ps){ return registerSequenceRecorder(ps); }

    ControlGUI & registerSynthRecorder(SynthRecorder &recorder);

    /// Register a SynthRecorder. This will display GUI widgets to control it
    ControlGUI & operator<< (SynthRecorder &recorder) { return registerSynthRecorder(recorder);}


    ControlGUI & registerSynthSequencer(SynthSequencer &seq);

    /// Register a SynthSequencer. This will display GUI widgets to control it
    /// Will also register the PolySynth contained within it.
    ControlGUI & operator<< (SynthSequencer &seq) { return registerSynthSequencer(seq); }


    ControlGUI & registerDynamicScene(DynamicScene &scene);

    /// Register a SynthSequencer. This will display GUI widgets to control it
    /// Will also register the PolySynth contained within it.
    ControlGUI & operator<< (DynamicScene &scene) { return registerDynamicScene(scene); }

    ControlGUI & registerMarker(GUIMarker &marker);

    /// Register a SynthSequencer. This will display GUI widgets to control it
    /// Will also register the PolySynth contained within it.
    ControlGUI & operator<< (GUIMarker marker) { return registerMarker(marker); }

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

    /**
     * @brief get currently active index for a parameter bundle registerd with the GUI
     * @param bundleName
     * @return current index
     *
     * Returns -1 if bundleName is not a registered bundle name
     */
    int getBundleCurrent(std::string bundleName) {
        if (mCurrentBundle.find(bundleName) != mCurrentBundle.end()) {
            return mCurrentBundle[bundleName];
        }
        return -1;
    }

    /**
     * @brief Returns true if global controls is enabled for bundle
     * @param bundleName name of the bundle to query
     */
    bool getBundleIsGlobal(std::string bundleName) {
        if (mBundleGlobal.find(bundleName) != mBundleGlobal.end()) {
            return mBundleGlobal[bundleName];
        }
        return false;
    }

    /**
     * @brief Set background alpha value
     *
     * 0 for transparent, 1 for opaque
     */
    void backgroundAlpha(float a) { mGUIBackgroundAlpha = a; }

    /**
     * @brief Get background alpha value
     *
     * 0 for transparent, 1 for opaque
     */
    float backgroundAlpha() const { return mGUIBackgroundAlpha; }

    static GUIMarker beginGroup(const char *groupName = nullptr) { return GUIMarker(GUIMarker::MarkerType::GROUP_BEGIN, groupName);}
    static GUIMarker endGroup() { return GUIMarker(GUIMarker::MarkerType::GROUP_END);}
    static GUIMarker separator() { return GUIMarker(GUIMarker::MarkerType::SEPARATOR);}

    static void drawParameterMeta(ParameterMeta *param, std::string suffix);
    static void drawParameter(Parameter *param, std::string suffix);
    static void drawParameterString(ParameterString *param, std::string suffix);
    static void drawParameterBool(ParameterBool *param, std::string suffix);
    static void drawParameterPose(ParameterPose *param, std::string suffix);
    static void drawParameterColor(ParameterColor *param, std::string suffix);
    static void drawMenu(ParameterMenu *param, std::string suffix);
    static void drawChoice(ParameterChoice *param, std::string suffix);
    static void drawVec3(ParameterVec3 *param, std::string suffix);
    static void drawVec4(ParameterVec4 *param, std::string suffix);

    static void drawParameterMeta(std::vector<ParameterMeta *> params, std::string suffix, int index = 0);
    static void drawParameter(std::vector<Parameter *> params, std::string suffix, int index = 0);
    static void drawParameterString(std::vector<ParameterString *> params, std::string suffix, int index = 0);
    static void drawParameterBool(std::vector<ParameterBool *> params, std::string suffix, int index = 0);
    static void drawParameterPose(std::vector<ParameterPose *> params, std::string suffix, int index = 0);
    static void drawParameterColor(std::vector<ParameterColor *> params, std::string suffix, int index = 0);
    static void drawMenu(std::vector<ParameterMenu *> params, std::string suffix, int index = 0);
    static void drawChoice(std::vector<ParameterChoice *> params, std::string suffix, int index = 0);
    static void drawVec3(std::vector<ParameterVec3 *> params, std::string suffix, int index = 0);
    static void drawVec4(std::vector<ParameterVec4 *> params, std::string suffix, int index = 0);

    static void drawDynamicScene(DynamicScene *scene, std::string suffix);

protected:

private:
    //std::map<std::string, std::vector<Parameter *>> mParameters;
    //std::map<std::string, std::vector<ParameterBool *>> mParameterBools;
    std::map<std::string, std::vector<ParameterVec3 *>> mParameterVec3s;
    std::map<std::string, std::vector<ParameterVec4 *>> mParameterVec4s;

	std::map<std::string, std::vector<ParameterMeta *>> mElements;
    ParameterMeta *mLatestElement {nullptr};
    std::vector<ParameterMeta *> mGroupBeginAnchors; // refs to the parameters marking beginning and ending of groups
    std::vector<std::string> mGroupNames;
    std::vector<ParameterMeta *> mGroupEndAnchors; // refs to the parameters marking beginning and ending of groups
    std::vector<ParameterMeta *> mSeparatorAnchors; 

    PresetHandler *mPresetHandler {nullptr};
    int mPresetColumns, mPresetRows;
    PresetSequencer *mPresetSequencer {nullptr};
    SequenceRecorder *mSequenceRecorder {nullptr};
    SynthRecorder *mSynthRecorder {nullptr};
    SynthSequencer *mSynthSequencer {nullptr};
    PolySynth *mPolySynth {nullptr};
    DynamicScene *mScene {nullptr};
    std::map<std::string, std::vector<ParameterBundle *>> mBundles;
    Nav *mNav {nullptr};

    int mX, mY;
    int mId;

    float mGUIBackgroundAlpha = 0;

    bool mManageIMGUI {true};

    int mCurrentSequencerItem {0};
    int mCurrentPresetSequencerItem {0};
    bool mStoreButtonOn {false};
    bool mRecordButtonValue {false};
    bool mOverwriteButtonValue {true};
    char** mSequencerItems;

    std::map<std::string, int> mCurrentBundle;
    std::map<std::string, bool> mBundleGlobal;

    // For use in ImGui::Combo
    static auto vector_getter(void* vec, int idx, const char** out_text)
    {
        auto& vector = *static_cast<std::vector<std::string>*>(vec);
        if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
        *out_text = vector.at(idx).c_str();
        return true;
    }

    void drawPresetHandler();
    void drawSequenceRecorder();

    void drawBundleGroup(std::vector<ParameterBundle *> bundles, std::string suffix);

    void drawNav();

};

}

#endif
