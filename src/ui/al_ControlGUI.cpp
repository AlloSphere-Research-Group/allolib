#include "al/ui/al_ControlGUI.hpp"

#include "al/graphics/al_Graphics.hpp"
#include "al/ui/al_SequenceRecorder.hpp"

using namespace al;
using namespace std;

void ControlGUI::draw(Graphics & /*g*/) {
  auto separatorAnchor = mSeparatorAnchors.begin();
  auto groupBeginAnchor = mGroupBeginAnchors.begin();
  auto groupNamesIt = mGroupNames.begin();
  auto groupEndAnchor = mGroupEndAnchors.begin();

  if (mManageIMGUI) {
    begin();
    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Header] = ImVec4(0.80f, 0.69f, 0.00f, 0.53f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.80f, 0.69f, 0.00f, 0.53f);
  } else {
    ImGui::SetNextWindowBgAlpha(mGUIBackgroundAlpha);
    ParameterGUI::beginPanel(mName, mX, mY);
  }
  ImGui::PushID(mName.c_str());
  if (mNav) {
    ParameterGUI::drawNav(mNav, mName);
  }
  if (mPresetHandler) {
    ParameterGUI::drawPresetHandler(mPresetHandler, mPresetColumns,
                                    mPresetRows);
  }
  if (mPresetSequencer) {
    ParameterGUI::drawPresetSequencer(mPresetSequencer,
                                      mCurrentPresetSequencerItem);
  }
  if (mSequenceRecorder) {
    ParameterGUI::drawSequenceRecorder(mSequenceRecorder);
  }
  if (mPolySynth) {
    if (ImGui::CollapsingHeader("PolySynth",
                                ImGuiTreeNodeFlags_CollapsingHeader |
                                    ImGuiTreeNodeFlags_DefaultOpen)) {
      if (ImGui::Button("Panic (All notes off)")) {
        mPolySynth->allNotesOff();
      }
    }
  }
  if (mSynthSequencer) {
    ParameterGUI::drawSynthSequencer(mSynthSequencer);
  }
  if (mSynthRecorder) {
    ParameterGUI::drawSynthRecorder(mSynthRecorder);
  }

  for (auto bundleGroup : mBundles) {
    bundleGroup.second->drawBundleGUI();
  }

  vector<bool> groupsVisibleStack;
  for (auto elem : mElements) {
    if (elem.first == "" ||
        ImGui::CollapsingHeader(
            elem.first.c_str(),
            ImGuiTreeNodeFlags_CollapsingHeader |
                ImGuiTreeNodeFlags_DefaultOpen)) {  // ! to force open by
      string suffix;
      if (elem.first.size() > 0) {
        suffix = "##" + elem.first;
      }
      for (ParameterMeta *p : elem.second) {
        // We do a runtime check to determine the type of the parameter to
        // determine how to draw it.
        if (groupsVisibleStack.size() == 0 ||
            groupsVisibleStack.back() == true) {
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
            groupsVisibleStack.push_back(ImGui::CollapsingHeader(
                (*groupNamesIt++ + suffix + "__group_" + p->getName()).c_str(),
                ImGuiTreeNodeFlags_CollapsingHeader |
                    ImGuiTreeNodeFlags_DefaultOpen));
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
  ImGui::PopID();
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
  if (mName == "") {
    mName = "ControlGUI_" + std::to_string(winCounter++);
  }

  mSequencerItems = (char **)malloc(32 * sizeof(char *));
  for (size_t i = 0; i < 32; i++) {
    mSequencerItems[i] = (char *)malloc(32 * sizeof(char));
  }

  if (mManageIMGUI) {
    imguiInit();
  }
}

void ControlGUI::begin() {
  ImGui::SetNextWindowPos(ImVec2(mX, mY));
  //  ImGui::SetNextWindowSize(ImVec2(width(), height()));
  ImGui::SetNextWindowBgAlpha(mGUIBackgroundAlpha);
  imguiBeginFrame();
  ImGui::Begin(mName.c_str(), nullptr, ImGuiWindowFlags_None);
}

void ControlGUI::end() {
  ImGui::End();
  if (mManageIMGUI) {
    imguiEndFrame();
    imguiDraw();
  }
}

void ControlGUI::cleanup() {
  if (mManageIMGUI) {
    imguiShutdown();
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

ControlGUI &ControlGUI::registerParameterBundle(ParameterBundle &bundle) {
  std::string bundleName = bundle.name();
  if (mBundles.find(bundleName) == mBundles.end()) {
    mBundles[bundleName] = new BundleGUIManager;
  }
  mBundles[bundleName]->registerParameterBundle(bundle);
  return *this;
}

ControlGUI &ControlGUI::registerNav(Nav &nav) {
  mNav = &nav;
  return *this;
}

ControlGUI &ControlGUI::registerPresetHandler(PresetHandler &presetHandler,
                                              int numRows, int numColumns) {
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

ControlGUI &ControlGUI::registerPresetSequencer(
    PresetSequencer &presetSequencer) {
  mPresetSequencer = &presetSequencer;
  return *this;
}

ControlGUI &ControlGUI::registerSequenceRecorder(SequenceRecorder &recorder) {
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
  switch (marker.getType()) {
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
