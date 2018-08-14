#include "al/util/ui/al_ControlGUI.hpp"

using namespace al;
using namespace std;

void ControlGUI::draw(Graphics &g) {

    if (mManageIMGUI) {
        begin();
    }
//    ImGui::SetNextWindowSize(ImVec2(300, 450), ImGuiCond_FirstUseEver);
//    ImGui::SetNextWindowPos(ImVec2(mX, mY), ImGuiCond_FirstUseEver);
//    ImGui::Begin(std::to_string(mId).c_str());
    if (mNav) {
		drawNav();
    }
    if (mPresetHandler) {
        if (ImGui::CollapsingHeader("Presets", ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen)) {

            std::map<int, std::string> presets = mPresetHandler->availablePresets();
            static int selection = -1;
            std::string currentPresetName = mPresetHandler->getCurrentPresetName();

            char buf1[64];
            strncpy(buf1, currentPresetName.c_str(), 64);
            if (ImGui::InputText("test", buf1, 64)) {
                currentPresetName = buf1;
            }
            static int presetHandlerBank = 0;
            int numColumns = 12;
            int numRows = 4;
            int counter = presetHandlerBank * (numColumns * numRows) ;
            std::string suffix = "##Preset"; 
            for (int row = 0; row < numRows; row++) {
                for (int column = 0; column < numColumns; column++) {
                    std::string name = std::to_string(counter);
                    ImGui::PushID(counter);

                    bool is_selected = selection == counter;
                    if (is_selected) {
                        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1, 0.1, 0.1, 1.0));
                    }
                    if (ImGui::Selectable((name + suffix).c_str(), is_selected, 0, ImVec2(18, 15)))
                    {
                        if (mStoreButtonOn) {
                            mPresetHandler->storePreset(counter, name.c_str());
                            selection = counter;
                            mStoreButtonOn = false;
                        } else {
                            if (mPresetHandler->recallPreset(counter) != "") { // Preset is available
                                selection = counter;
                            }
                        }
                    }
                    if (is_selected) {
                        ImGui::PopStyleColor(1);
                    }
                    if (column < numColumns - 1) ImGui::SameLine();
                    counter++;
                    ImGui::PopID();
                }
            }
            ImGui::Checkbox("Store##Presets", &mStoreButtonOn);
            ImGui::SameLine();
            static std::vector<string> seqList {"1", "2", "3", "4"};
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
            ImGui::Combo("Bank##PresetSequencer", &presetHandlerBank, vector_getter, static_cast<void*>(&seqList), seqList.size());
            ImGui::PopItemWidth();
//            ImGui::Text("%s", currentPresetName.c_str());

        }
    }
    if (mPresetSequencer) {
        if (ImGui::CollapsingHeader("Preset Sequencer", ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen )) {
            vector<string> seqList = mPresetSequencer->getSequenceList();
            if (seqList.size() > 0) {
                if (seqList.size() > 64) {
                    seqList.resize(64);
                    std::cout << "Cropping sequence list to 64 items for display" <<std::endl;
                }
                // for (size_t i = 0; i < seqList.size(); i++) {
                //     strncpy(mSequencerItems[i], seqList[i].c_str(), 32);
                // }
                // int items_count = seqList.size();

                ImGui::Combo("Sequences##PresetSequencer", &mCurrentPresetSequencerItem, vector_getter,
                            static_cast<void*>(&seqList), seqList.size());
                if (ImGui::Button("Play##PresetSequencer")) {
                    mPresetSequencer->playSequence(seqList[mCurrentSequencerItem]);
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop##PresetSequencer")) {
                    mPresetSequencer->stopSequence();
                }

                // for (size_t i = 0; i < seqList.size(); i++) {
                //     //                free(items[i]);
                // }
            }
        }
    }
    if (mPolySynth) {
        if (ImGui::CollapsingHeader("PolySynth", ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("Panic (All notes off)")) {
                mPolySynth->allNotesOff();
            }
        }
    }
    if (mSynthSequencer) {
        if (ImGui::CollapsingHeader("Event Sequencer##EventSequencer", ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen)) {
            vector<string> seqList = mSynthSequencer->getSequenceList();
            if (seqList.size() > 0) {
                if (seqList.size() > 64) {
                    seqList.resize(64);
                    std::cout << "Cropping sequence list to 64 items for display" <<std::endl;
                }
                // for (size_t i = 0; i < seqList.size(); i++) {
                //     strncpy(mSequencerItems[i], seqList[i].c_str(), 32);
                // }
                // int items_count = seqList.size();
                ImGui::Combo("Sequences##EventSequencer", &mCurrentSequencerItem, vector_getter,
                            static_cast<void*>(&seqList), seqList.size());
                if (ImGui::Button("Play##EventSequencer")) {
                    mSynthSequencer->synth().allNotesOff();
                    mSynthSequencer->playSequence(seqList[mCurrentSequencerItem]);
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop##EventSequencer")) {
                    mSynthSequencer->stopSequence();
                    mSynthSequencer->synth().allNotesOff();
                }

                for (size_t i = 0; i < seqList.size(); i++) {
                    //                free(items[i]);
                }
            }
        }
    }
    if (mSynthRecorder) {
        if (ImGui::CollapsingHeader("Event Recorder##EventRecorder", ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen)) {
            static char buf1[64] = "test"; ImGui::InputText("Record Name", buf1, 64);
            if (ImGui::Checkbox("Record##EventRecorder", &mRecordButtonValue)) {
                if (mRecordButtonValue) {
                    mSynthRecorder->startRecord(buf1, mOverwriteButtonValue);
                } else {
                    mSynthRecorder->stopRecord();
                }
            }
            ImGui::SameLine();
            ImGui::Checkbox("Overwrite", &mOverwriteButtonValue);
        }
    }
//    ImGui::ShowDemoWindow();
    for (auto elem: mElements) {
        if(elem.first == "" || ImGui::CollapsingHeader(elem.first.c_str(), ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen)) { // ! to force open by 
            string suffix;
            if (elem.first.size() > 0){
                suffix = "##" + elem.first;
            }

			for (ParameterMeta *p: elem.second) {
				// We do a runtime check to determine the type of the parameter to determine how to draw it.
				if (strcmp(typeid(*p).name(), typeid(ParameterBool).name() ) == 0) { // ParameterBool
					ParameterBool *param = dynamic_cast<ParameterBool *>(p);

					drawParameterBool(param, suffix);
				} else if (strcmp(typeid(*p).name(), typeid(Parameter).name()) == 0) {// Parameter
					Parameter *param = dynamic_cast<Parameter *>(p);
					drawParameter(param, suffix);
				} else if (strcmp(typeid(*p).name(), typeid(ParameterPose).name()) == 0) {// Parameter
					ParameterPose *param = dynamic_cast<ParameterPose *>(p);
					drawParameterPose(param);
				} else if (strcmp(typeid(*p).name(), typeid(ParameterMenu).name()) == 0) {// Parameter
					ParameterMenu *param = dynamic_cast<ParameterMenu *>(p);
					drawMenu(param, suffix);
				}
				else {
					// TODO this check should be performed on registration
					std::cout << "Unsupported Parameter type for display" << std::endl;
				}
            }

        }
    }
//    ImGui::End(); // End the window
    if (mManageIMGUI) {
        end();
    }
}

void ControlGUI::init(int x, int y) {
    static int id = 0;
    mId = id++;
    mX = x;
    mY = y;

    mSequencerItems = (char **) malloc(32 * sizeof(char *));
    for (size_t i = 0; i < 32; i++) {
        mSequencerItems[i] = (char *) malloc(32 * sizeof(char));
    }

    if (mManageIMGUI) {
        initIMGUI();
    }
}

void ControlGUI::begin() {
    string name = "__ControlGUI_" + std::to_string(mId);
    beginIMGUI_minimal(true, name.c_str(), mX, mY, mGUIBackgroundAlpha);
}

void ControlGUI::end() {
    endIMGUI_minimal(true);
}

void ControlGUI::cleanup() {
    if (mManageIMGUI) {
        shutdownIMGUI();
    }
    for (size_t i = 0; i < 32; i++) {
        free(mSequencerItems[i]);
    }
    free(mSequencerItems);
}

void ControlGUI::drawParameter(Parameter * param, string suffix)
{
	if (param->getHint("intcombo") == 1.0) {
		int value = (int)param->get();
		vector<string> values;
		for (float i = param->min(); i <= param->max(); i++) {
			// There's got to be a better way...
			values.push_back(to_string((int)i));
		}
		auto vector_getter = [](void* vec, int idx, const char** out_text)
		{
			auto& vector = *static_cast<std::vector<std::string>*>(vec);
			if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
			*out_text = vector.at(idx).c_str();
			return true;
		};
		if (!values.empty()) {
			bool changed = ImGui::Combo((param->getName() + suffix).c_str(), &value, vector_getter,
				static_cast<void*>(&values), values.size());
			if (changed) {
				param->set(value);
			}
		}
	}
	else {
		float value = param->get();
		bool changed = ImGui::SliderFloat((param->getName() + suffix).c_str(), &value, param->min(), param->max());
		if (changed) {
			param->set(value);
		}
	}
}

void ControlGUI::drawParameterBool(ParameterBool * param, string suffix)
{
	bool changed;
	if (param->getHint("latch") == 1.0) {
		bool value = param->get() == 1.0;
		changed = ImGui::Checkbox((param->getName() + suffix).c_str(), &value);
		if (changed) {
			param->set(value ? 1.0 : 0.0);
		}
	}
	else {
		changed = ImGui::Button((param->getName() + suffix).c_str());
		if (changed) {
			param->set(1.0);
		}
		else {
			if (param->get() == 1.0) {
				param->set(0.0);
			}
		}
	}
}

void ControlGUI::drawParameterPose(ParameterPose *pose)
{
	if (ImGui::CollapsingHeader(("Pose:" + pose->getName()).c_str(), ImGuiTreeNodeFlags_CollapsingHeader)) {
		Vec3d &currentPos = pose->get().pos();
		float x = currentPos.elems()[0];
		if (ImGui::SliderFloat(("X##" + pose->getName()).c_str(), &x, -5, 5)) {
			currentPos.elems()[0] = x;
			pose->set(Pose(currentPos, pose->get().quat()));
		}
		float y = currentPos.elems()[1];
		if (ImGui::SliderFloat(("Y##" + pose->getName()).c_str(), &y, -5, 5)) {
			currentPos.elems()[1] = y;
			pose->set(Pose(currentPos, pose->get().quat()));
		}
		float z = currentPos.elems()[2];
		if (ImGui::SliderFloat(("Z##" + pose->getName()).c_str(), &z, -10, 0)) {
			currentPos.elems()[2] = z;
			pose->set(Pose(currentPos, pose->get().quat()));
		}
		ImGui::Spacing();
	}
}

void ControlGUI::drawNav()
{
	if (ImGui::CollapsingHeader("Navigation##nav", ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen)) {
		Vec3d &currentPos = mNav->pos();
		float x = currentPos.elems()[0];

		bool changed = ImGui::SliderFloat("X", &x, -10, 10);
		if (changed) {
			currentPos.elems()[0] = x;
		}
		float y = currentPos.elems()[1];
		changed = ImGui::SliderFloat("Y", &y, -10, 10);
		if (changed) {
			currentPos.elems()[1] = y;
		}
		float z = currentPos.elems()[2];
		changed = ImGui::SliderFloat("Z", &z, -10, 10);
		if (changed) {
			currentPos.elems()[2] = z;
		}
		ImGui::Spacing();
	}
}

void al::ControlGUI::drawMenu(ParameterMenu * param, std::string suffix)
{
	int value = param->get();
	auto values = param->getElements();
	bool changed = ImGui::Combo((param->getName() + suffix).c_str(), &value, vector_getter,
				static_cast<void*>(&values), values.size());
	if (changed) {
		param->set(value);
	}
}

ControlGUI &ControlGUI::registerParameterMeta(ParameterMeta &param) {
    std::string group = param.getGroup();
    if (mElements.find(group) == mElements.end()) {
		mElements[group] = std::vector<ParameterMeta *>();
    }
	mElements[group].push_back(&param);
    return *this;
}

ControlGUI &ControlGUI::registerNav(Nav &nav)
{
    mNav = &nav;
    return *this;
}

ControlGUI &ControlGUI::registerPresetHandler(PresetHandler &presetHandler) {
    mPresetHandler = &presetHandler;
    return *this;
}

ControlGUI &ControlGUI::registerPresetSequencer(PresetSequencer &presetSequencer) {
    mPresetSequencer = &presetSequencer;
    return *this;
}

void ControlGUI::registerSynthRecorder(SynthRecorder &recorder) {
    mSynthRecorder = &recorder;
}

void ControlGUI::registerSynthSequencer(SynthSequencer &seq) {
    mSynthSequencer = &seq;
    mPolySynth = &seq.synth();
}