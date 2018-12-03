#ifndef AL_PARAMETERGUI_H
#define AL_PARAMETERGUI_H

/*	Allolib --
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
    Expose parameters on the network
	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include "al/core/io/al_ControlNav.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/util/ui/al_Preset.hpp"
#include "al/util/ui/al_PresetSequencer.hpp"
#include "al/util/ui/al_ParameterBundle.hpp"
#include "al/util/ui/al_PresetSequencer.hpp"
#include "al/util/ui/al_SequenceRecorder.hpp"
#include "al/util/scene/al_SynthSequencer.hpp"
#include "al/util/scene/al_SynthRecorder.hpp"
#include "al/util/scene/al_DynamicScene.hpp"
#include "al/util/imgui/al_Imgui.hpp"

namespace al {

class ParameterGUI {
public:

    // These functions require no state other than the parameter itself
    static void drawParameterMeta(ParameterMeta *param, std::string suffix = "");
    static void drawParameter(Parameter *param, std::string suffix = "");
    static void drawParameterString(ParameterString *param, std::string suffix = "");
    static void drawParameterInt(ParameterInt *param, std::string suffix);
    static void drawParameterBool(ParameterBool *param, std::string suffix = "");
    static void drawParameterPose(ParameterPose *param, std::string suffix = "");
    static void drawParameterColor(ParameterColor *param, std::string suffix = "");
    static void drawMenu(ParameterMenu *param, std::string suffix = "");
    static void drawChoice(ParameterChoice *param, std::string suffix = "");
    static void drawVec3(ParameterVec3 *param, std::string suffix = "");
    static void drawVec4(ParameterVec4 *param, std::string suffix = "");

    // Display for al types
    static void drawNav(Nav *mNav, std::string suffix = "");
    static void drawDynamicScene(DynamicScene *scene, std::string suffix = "");

    // These functions are for use in bundles to only display one from a group of parameters
    static void drawParameterMeta(std::vector<ParameterMeta *> params, std::string suffix, int index = 0);
    static void drawParameter(std::vector<Parameter *> params, std::string suffix, int index = 0);
    static void drawParameterString(std::vector<ParameterString *> params, std::string suffix, int index = 0);
    static void drawParameterInt(std::vector<ParameterInt *> params, std::string suffix, int index = 0);
    static void drawParameterBool(std::vector<ParameterBool *> params, std::string suffix, int index = 0);
    static void drawParameterPose(std::vector<ParameterPose *> params, std::string suffix, int index = 0);
    static void drawParameterColor(std::vector<ParameterColor *> params, std::string suffix, int index = 0);
    static void drawMenu(std::vector<ParameterMenu *> params, std::string suffix, int index = 0);
    static void drawChoice(std::vector<ParameterChoice *> params, std::string suffix, int index = 0);
    static void drawVec3(std::vector<ParameterVec3 *> params, std::string suffix, int index = 0);
    static void drawVec4(std::vector<ParameterVec4 *> params, std::string suffix, int index = 0);

    // These functions require additional state that is passed as reference

    static void drawPresetHandler(PresetHandler *presetHandler, int presetColumns, int presetRows, bool &storeButtonOn);
    static void drawSequenceRecorder(SequenceRecorder *sequenceRecorder, bool &overwriteButtonValue);

    void drawBundleGroup(std::vector<ParameterBundle *> bundles, std::string suffix, std::map<std::string, int> &currentBundle, std::map<std::string, bool> &bundleGlobal);

    // Convenience function for use in ImGui::Combo
    static auto vector_getter(void* vec, int idx, const char** out_text)
    {
        auto& vector = *static_cast<std::vector<std::string>*>(vec);
        if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
        *out_text = vector.at(idx).c_str();
        return true;
    }

    static void beginPanel(std::string name, float x = -1, float y = -1, float width = 0, float height = 0,
                      ImGuiWindowFlags window_flags = 0) {
        if (x >= 0 || y >= 0) {
            ImGui::SetNextWindowSize(ImVec2(width, height));
        }
        ImGui::SetNextWindowPos(ImVec2(x, y));
//        ImGuiWindowFlags window_flags = 0;
//        window_flags |= ImGuiWindowFlags_NoTitleBar; // if (no_titlebar)
        // window_flags |= ImGuiWindowFlags_NoScrollbar; // if (no_scrollbar)
        // window_flags |= ImGuiWindowFlags_MenuBar; // if (!no_menu)
//        window_flags |= ImGuiWindowFlags_NoMove; // if (no_move)
//        window_flags |= ImGuiWindowFlags_NoResize; // if (no_resize)
//        window_flags |= ImGuiWindowFlags_NoCollapse; // if (no_collapse)
//        window_flags |= ImGuiWindowFlags_NoNav; // if (no_nav)

        ImGui::Begin(name.c_str() , nullptr, window_flags);
    }

    static void endPanel() {
        ImGui::End();
    }


};

} // namespace al

#endif //AL_PARAMETERGUI_H
