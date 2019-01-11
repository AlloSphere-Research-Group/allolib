#include "al/util/ui/al_ControlGUI.hpp"

#include "al/util/ui/al_SequenceRecorder.hpp"

using namespace al;
using namespace std;

void ControlGUI::draw(Graphics &g) {
    auto separatorAnchor = mSeparatorAnchors.begin();
    auto groupBeginAnchor = mGroupBeginAnchors.begin();
    auto groupNamesIt = mGroupNames.begin();
    auto groupEndAnchor = mGroupEndAnchors.begin();


    if (mManageIMGUI) {
        begin();
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_Header] = ImVec4(0.80f, 0.69f, 0.00f, 0.53f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.80f, 0.69f, 0.00f, 0.53f);
    } else {
        ImGui::SetNextWindowBgAlpha(mGUIBackgroundAlpha);
        ParameterGUI::beginPanel(mName, mX, mY);
    }
//    ImGui::SetNextWindowSize(ImVec2(300, 450), ImGuiCond_FirstUseEver);
//    ImGui::SetNextWindowPos(ImVec2(mX, mY), ImGuiCond_FirstUseEver);
//    ImGui::Begin(std::to_string(mId).c_str());
    if (mNav) { ParameterGUI::drawNav(mNav, mName);}
    if (mPresetHandler) {
        ParameterGUI::drawPresetHandler(mPresetHandler, mPresetColumns, mPresetRows);
    }
    if (mPresetSequencer) {
        ParameterGUI::drawPresetSequencer(mPresetSequencer, mCurrentPresetSequencerItem);
    }
    if (mSequenceRecorder) {
        ParameterGUI::drawSequenceRecorder(mSequenceRecorder,
                                           mOverwriteButtonValue);
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
                ImGui::Combo("Sequences##EventSequencer", &mCurrentSequencerItem, ParameterGUI::vector_getter,
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
        if (ImGui::CollapsingHeader("Event Recorder##__EventRecorder", ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen)) {
            static char buf1[64] = "test"; ImGui::InputText("Record Name##__EventRecorder", buf1, 64);
            if (ImGui::Checkbox("Record##__EventRecorder", &mRecordButtonValue)) {
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

    for (auto bundleGroup: mBundles) {
        bundleGroup.second->drawBundleGUI();
//        mParameterGUI.drawBundleGroup(bundleGroup.second, suffix,
//                                      mCurrentBundle[bundleGroup.first], mBundleGlobal[bundleGroup.first]);
    }


//    ImGui::ShowDemoWindow();
    vector<bool> groupsVisibleStack;
    for (auto elem: mElements) {
        if(elem.first == "" || ImGui::CollapsingHeader(elem.first.c_str(), ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen)) { // ! to force open by 
            string suffix;
            if (elem.first.size() > 0){
                suffix = "##" + elem.first;
            }

			for (ParameterMeta *p: elem.second) {
				// We do a runtime check to determine the type of the parameter to determine how to draw it.
                if (groupsVisibleStack.size() == 0 || groupsVisibleStack.back() == true) {
                    ParameterGUI::drawParameterMeta(p, suffix);
                    if (separatorAnchor != mSeparatorAnchors.end()) {
                        // The spacing's visibility depends on its position,
                        // So here we show it, but we need to do the increment
                        // below outside the visibility check
                        if (*separatorAnchor == p) {
                            ImGui::Separator();
                        }
                    }
                }
                if (separatorAnchor != mSeparatorAnchors.end()) {
                    if (*separatorAnchor == p) {
                        separatorAnchor++;
                    }
                }
                if (groupBeginAnchor != mGroupBeginAnchors.end()) {
                    if (*groupBeginAnchor == p || *groupBeginAnchor == nullptr) {
                        groupsVisibleStack.push_back(ImGui::CollapsingHeader((*groupNamesIt++ + suffix + "__group_" + p->getName()).c_str() ,
                                    ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen));
                        groupBeginAnchor++;
                    }
                }
                if (groupEndAnchor != mGroupEndAnchors.end()) {
                    if (*groupEndAnchor == p) {
                        if (groupsVisibleStack.back() == true) {
                            // If group is visible add a spacing to mark the end of the group
                            ImGui::Separator();
                        }
                        groupsVisibleStack.pop_back();
                        groupEndAnchor++;
                    }
                }
            }

        }
    }
//    ImGui::End(); // End the window
    if (mManageIMGUI) {
        end();
    } else {

        ParameterGUI::endPanel();
    }
}

void ControlGUI::init(int x, int y, bool manageImgui) {
    static int winCounter = 0;
    mX = x;
    mY = y;
    mManageIMGUI = manageImgui;
    mName = "__ControlGUI_" + std::to_string(winCounter++);

    mSequencerItems = (char **) malloc(32 * sizeof(char *));
    for (size_t i = 0; i < 32; i++) {
        mSequencerItems[i] = (char *) malloc(32 * sizeof(char));
    }

    if (mManageIMGUI) {
        initIMGUI();
    }
}

void ControlGUI::begin() {
    beginIMGUI_minimal(true, mName.c_str(), mX, mY, mGUIBackgroundAlpha);
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

ControlGUI &ControlGUI::registerParameterMeta(ParameterMeta &param) {
    std::string group = param.getGroup();
    if (mElements.find(group) == mElements.end()) {
		mElements[group] = std::vector<ParameterMeta *>();
    }
	mElements[group].push_back(&param);
    mLatestElement = &param;
    return *this;
}

ControlGUI &ControlGUI::registerParameterBundle(ParameterBundle &bundle)
{
    std::string bundleName = bundle.name();
    if (mBundles.find(bundleName) == mBundles.end()) {
        mBundles[bundleName] = new BundleGUIManager;
    }
    mBundles[bundleName]->registerParameterBundle(bundle);
    return *this;
}

ControlGUI &ControlGUI::registerNav(Nav &nav)
{
    mNav = &nav;
    return *this;
}

ControlGUI &ControlGUI::registerPresetHandler(PresetHandler &presetHandler, int numRows, int numColumns) {
    mPresetHandler = &presetHandler;
    if (numRows == -1) {
        mPresetRows = 4;
    } else {
        mPresetRows = numRows;
    }

    if (numColumns == -1) {
        mPresetColumns = 12;
    } else {
        mPresetColumns = numColumns;
    }
    return *this;
}

ControlGUI &ControlGUI::registerPresetSequencer(PresetSequencer &presetSequencer) {
    mPresetSequencer = &presetSequencer;
    return *this;
}

ControlGUI &ControlGUI::registerSequenceRecorder(SequenceRecorder &recorder)
{
    mSequenceRecorder = &recorder;
    return *this;
}

ControlGUI &ControlGUI::registerSynthRecorder(SynthRecorder &recorder) {
    mSynthRecorder = &recorder;
    return *this;
}

ControlGUI &ControlGUI::registerSynthSequencer(SynthSequencer &seq) {
    mSynthSequencer = &seq;
    mPolySynth = &seq.synth();
    return *this;
}

ControlGUI &ControlGUI::registerDynamicScene(DynamicScene &scene) {
    mScene = &scene;
    return *this;
}

ControlGUI &ControlGUI::registerMarker(GUIMarker &marker) {
    switch(marker.getType()) {
        case GUIMarker::MarkerType::GROUP_BEGIN:
        mGroupBeginAnchors.push_back(mLatestElement);
        mGroupNames.push_back(marker.getName());
        break;
        case GUIMarker::MarkerType::GROUP_END:
        mGroupEndAnchors.push_back(mLatestElement);
        break;
        case GUIMarker::MarkerType::SEPARATOR:
        mSeparatorAnchors.push_back(mLatestElement);
        break;
    }
    return *this;
}

