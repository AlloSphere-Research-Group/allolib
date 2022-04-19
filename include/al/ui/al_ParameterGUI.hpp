#ifndef AL_PARAMETERGUI_H
#define AL_PARAMETERGUI_H

/*	Allolib --
   Multimedia / virtual environment application class library

   Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012-2018. The Regents of the University of California.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   Neither the name of the University of California nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

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
   Expose parameters on the network
   File author(s):
   Andrés Cabrera mantaraya36@gmail.com
*/

#include <climits>

#include "al/io/al_AudioIO.hpp"
#include "al/io/al_ControlNav.hpp"
#include "al/io/al_Imgui.hpp"
#include "al/scene/al_DynamicScene.hpp"
#include "al/scene/al_SynthRecorder.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterBundle.hpp"
#include "al/ui/al_ParameterMIDI.hpp"
#include "al/ui/al_PresetHandler.hpp"
#include "al/ui/al_PresetMIDI.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al/ui/al_SequenceRecorder.hpp"

namespace al {

class BundleGUIManager;

/// ParameterGUI
/// @ingroup UI
class ParameterGUI {
public:
  //    static void init() { imguiInit(); }
  //    static void cleanup() { imguiShutdown(); }

  static inline void draw(ParameterMeta *param) { drawParameterMeta(param); }

  // These functions require no state other than the parameter itself
  static void drawVectorParameters(std::vector<ParameterMeta *> params,
                                   std::string suffix = "");
  static void drawParameterMeta(ParameterMeta *param, std::string suffix = "");
  static void drawParameter(Parameter *param, std::string suffix = "");
  static void drawParameterString(ParameterString *param,
                                  std::string suffix = "");
  static void drawParameterInt(ParameterInt *param, std::string suffix);
  static void drawParameterBool(ParameterBool *param, std::string suffix = "");
  static void drawParameterPose(ParameterPose *param, std::string suffix = "");
  static void drawParameterColor(ParameterColor *param,
                                 std::string suffix = "");
  static void drawMenu(ParameterMenu *param, std::string suffix = "");
  static void drawChoice(ParameterChoice *param, std::string suffix = "");
  static void drawVec3(ParameterVec3 *param, std::string suffix = "");
  static void drawVec4(ParameterVec4 *param, std::string suffix = "");
  static void drawTrigger(Trigger *param, std::string suffix = "");

  static void drawSynthController(PolySynth *param, std::string suffix = "");

  // Display for al types
  static void drawNav(Nav *mNav, std::string suffix = "");
  static void drawDynamicScene(DynamicScene *scene, std::string suffix = "");

  // These functions are for use in bundles to only display one from a group of
  // parameters
  static void drawParameterMeta(std::vector<ParameterMeta *> params,
                                std::string suffix, int index = 0);
  static void drawParameter(std::vector<Parameter *> params, std::string suffix,
                            int index = 0);
  static void drawParameterString(std::vector<ParameterString *> params,
                                  std::string suffix, int index = 0);
  static void drawParameterInt(std::vector<ParameterInt *> params,
                               std::string suffix, int index = 0);
  static void drawParameterBool(std::vector<ParameterBool *> params,
                                std::string suffix, int index = 0);
  static void drawParameterPose(std::vector<ParameterPose *> params,
                                std::string suffix, int index = 0);
  static void drawParameterColor(std::vector<ParameterColor *> params,
                                 std::string suffix, int index = 0);
  static void drawMenu(std::vector<ParameterMenu *> params, std::string suffix,
                       int index = 0);
  static void drawChoice(std::vector<ParameterChoice *> params,
                         std::string suffix, int index = 0);
  static void drawVec3(std::vector<ParameterVec3 *> params, std::string suffix,
                       int index = 0);
  static void drawVec4(std::vector<ParameterVec4 *> params, std::string suffix,
                       int index = 0);
  static void drawTrigger(std::vector<Trigger *> params, std::string suffix,
                          int index = 0);

  // These functions require additional state that is passed as reference

  struct PresetHandlerState {
    std::string currentBank;
    int currentBankIndex = 0;
    std::vector<std::string> mapList;
    int presetHandlerBank = 0;
    bool newMap = false;
    std::string enteredText;
    std::string newMapText;
    bool storeButtonState{false};
  };

  static PresetHandlerState &drawPresetHandler(PresetHandler *presetHandler,
                                               int presetColumns = 10,
                                               int presetRows = 4);
  static void drawPresetSequencer(PresetSequencer *presetSequencer,
                                  int &currentPresetSequencerItem);
  static void drawSequenceRecorder(SequenceRecorder *sequenceRecorder);
  static void drawSynthSequencer(SynthSequencer *synthSequencer);
  static void drawSynthRecorder(SynthRecorder *synthRecorder);

  /**
   * @brief returns true if audio device has been restarted
   */
  static bool drawAudioIO(AudioIO &io) { return drawAudioIO(&io); }
  static bool drawAudioIO(AudioIO *io);
  static void drawParameterMIDI(ParameterMIDI &midi) {
    drawParameterMIDI(&midi);
  }

  static void drawParameterMIDI(ParameterMIDI *midi);
  static void drawPresetMIDI(PresetMIDI *presetMidi);
  static void drawMIDIIn(RtMidiIn *midiIn);

  static void drawBundleGroup(std::vector<ParameterBundle *> bundles,
                              std::string suffix, int &currentBundle,
                              bool &bundleGlobal);
  static void drawBundle(ParameterBundle *bundle);
  static void drawBundleManager(BundleGUIManager *manager);

  static bool usingInput() { return isImguiUsingInput(); }
  static bool usingKeyboard() { return isImguiUsingKeyboard(); }

  // Convenience function for use in ImGui::Combo
  static auto vector_getter(void *vec, int idx, const char **out_text) {
    auto &vector = *static_cast<std::vector<std::string> *>(vec);
    if (idx < 0 || idx >= static_cast<int>(vector.size())) {
      return false;
    }
    *out_text = vector.at(idx).c_str();
    return true;
  }

  static void beginPanel(std::string name, float x = -1, float y = -1,
                         float width = -1, float height = -1,
                         ImGuiWindowFlags window_flags = 0) {
    if (x >= 0 || y >= 0) {
      ImGui::SetNextWindowPos(ImVec2(x, y));
      //            window_flags |= ImGuiWindowFlags_NoMove; // if (no_move)
    }
    if (width >= 0 || height >= 0) {
      ImGui::SetNextWindowSize(ImVec2(width, height));
      //            window_flags |= ImGuiWindowFlags_NoResize; // if (no_resize)
    }
    //        ImGuiWindowFlags window_flags = 0;
    //        window_flags |= ImGuiWindowFlags_NoTitleBar; // if (no_titlebar)
    // window_flags |= ImGuiWindowFlags_NoScrollbar; // if (no_scrollbar)
    // window_flags |= ImGuiWindowFlags_MenuBar; // if (!no_menu)
    //        window_flags |= ImGuiWindowFlags_NoCollapse; // if (no_collapse)
    //        window_flags |= ImGuiWindowFlags_NoNav; // if (no_nav)

    ImGui::Begin(name.c_str(), nullptr, window_flags);
  }

  static void endPanel() { ImGui::End(); }
};

/// BundleGUIManager
/// @ingroup UI
class BundleGUIManager {
public:
  void drawBundleGUI() {
    std::unique_lock<std::mutex> lk(mBundleLock);
    std::string suffix = "##_bundle_" + mName;
    ParameterGUI::drawBundleGroup(mBundles, suffix, mCurrentBundle,
                                  mBundleGlobal);
  }

  BundleGUIManager &registerParameterBundle(ParameterBundle &bundle) {
    if (mName.size() == 0 || bundle.name() == mName) {
      std::unique_lock<std::mutex> lk(mBundleLock);
      if (mName.size() == 0) {
        mName = bundle.name();
      }
      mBundles.push_back(&bundle);
    } else {
      std::cout << "Warning: bundle name mismatch. Bundle '" << bundle.name()
                << "' ingnored." << std::endl;
    }
    return *this;
  }
  /// Register parameter using the streaming operator.
  BundleGUIManager &operator<<(ParameterBundle &newBundle) {
    return registerParameterBundle(newBundle);
  }

  /// Register parameter using the streaming operator.
  BundleGUIManager &operator<<(ParameterBundle *newBundle) {
    return registerParameterBundle(*newBundle);
  }

  std::string name() { return mName; }

  int &currentBundle() { return mCurrentBundle; }

  void setCurrentBundle(int index) {
    if (index >= 0) {
      mCurrentBundle = index;
    } else {
      mBundleGlobal = true;
    }
  }
  bool &bundleGlobal() { return mBundleGlobal; }
  std::vector<ParameterBundle *> bundles() { return mBundles; }

private:
  std::mutex mBundleLock;
  std::vector<ParameterBundle *> mBundles;
  std::string mName;
  int mCurrentBundle{0};
  bool mBundleGlobal{false};
};

/// SynthGUIManager
/// @ingroup UI
template <class VoiceType> class SynthGUIManager {
public:
  SynthGUIManager(std::string name = "") {
    mControlVoice.init();
    for (auto *param : mControlVoice.triggerParameters()) {
      mPresetHandler << *param;
    }
    //        mSynth = &synth;

    mName = name;
    if (mName == "") {
      mName = demangle(typeid(mControlVoice).name());
    }
    mPresetHandler.setRootPath(mName + "-data");
    mPresetSequencer << mPresetHandler;
    mPresetSequenceRecorder << mPresetHandler;

    mSequencer.setDirectory(mName + "-data");
    mRecorder.setDirectory(mName + "-data");
    //        mSequencer << *mSynth;
    mRecorder << mSequencer.synth();

    //        template<class VoiceType>
    mSequencer.synth().template registerSynthClass<VoiceType>(
        demangle(typeid(VoiceType).name()));
    mSequencer.synth().template allocatePolyphony<VoiceType>(16);
  }

  std::string name() { return mName; }

  void drawSynthWidgets() {
    drawFields();
    drawAllNotesOffButton();
    drawPresets();
    ImGui::Separator();
    ImGui::Columns(2, nullptr, true);
    if (ImGui::Selectable("Polyphonic", mCurrentTab == 1)) {
      mCurrentTab = 1;
      triggerOff();
    }
    ImGui::NextColumn();
    if (ImGui::Selectable("Static", mCurrentTab == 2)) {
      mCurrentTab = 2;
      synthSequencer().stopSequence();
      synth().allNotesOff();
      //            while (synth().getActiveVoices()) {} // Spin until all
      //            voices have been removed triggerOn();
    }

    ImGui::Columns(1);
    if (mCurrentTab == 1) {
      drawAllNotesOffButton();
      drawSynthSequencer();
      drawSynthRecorder();
    } else {
      drawTriggerButton();
      drawPresetSequencer();
      drawPresetSequencerRecorder();
    }
  }

  void setCurrentTab(int tab) {
    if (tab >= 1 && tab <= 2) {
      mCurrentTab = tab;
    } else {
      std::cerr << "ERROR: Can't set tab for SynthGUIManager:" << tab
                << std::endl;
    }
  }

  /**
   * @brief Draws a panel with all the synth controls.
   */
  void drawSynthControlPanel() {
    ParameterGUI::beginPanel(demangle(typeid(mControlVoice).name()).c_str());
    drawSynthWidgets();
    ParameterGUI::endPanel();
  }

  void render(AudioIOData &io) { synthSequencer().render(io); }

  void render(Graphics &g) { synthSequencer().render(g); }

  void configureVoiceFromGui(VoiceType *voice) {
    for (size_t i = 0; i < mControlVoice.triggerParameters().size(); i++) {
      voice->triggerParameters()[i]->set(mControlVoice.triggerParameters()[i]);
    }
  }

  void drawFields() {
    for (auto *param : mControlVoice.triggerParameters()) {
      ParameterGUI::drawParameterMeta(param);
    }
  }

  void drawPresets(int columns = 12, int rows = 4) {
    ParameterGUI::drawPresetHandler(&mPresetHandler, columns, rows);
  }

  void drawSynthSequencer() { ParameterGUI::drawSynthSequencer(&mSequencer); }

  void drawSynthRecorder() { ParameterGUI::drawSynthRecorder(&mRecorder); }

  void drawPresetSequencer() {
    static int currentItem{0};
    ParameterGUI::drawPresetSequencer(&mPresetSequencer, currentItem);
  }

  void drawPresetSequencerRecorder() {
    ParameterGUI::drawSequenceRecorder(&mPresetSequenceRecorder);
  }

  void createBundle(uint8_t bundleSize) {
    for (uint8_t i = 0; i < bundleSize; i++) {
      auto voice = mSequencer.synth().template getVoice<VoiceType>();
      auto bundle = std::make_shared<ParameterBundle>(
          demangle(typeid(mControlVoice).name()));
      for (auto *param : voice->triggerParameters()) {
        *bundle << param;
      }
      mBundles.push_back(bundle);
      mBundleGui << *bundle;
      mSequencer.synth().triggerOn(voice);
    }
  }

  void drawBundle() {
    if (mBundles.size() > 0) {
      mBundleGui.drawBundleGUI();
    }
  }

  void drawTriggerButton() {
    std::string buttonName =
        mCurrentTriggerState ? "Turn off##paramGUI" : "Trigger##paramGUI";
    if (ImGui::Button(buttonName.c_str(),
                      ImVec2(ImGui::GetWindowWidth(), 40))) {
      if (!mCurrentTriggerState) {
        triggerOn();
      } else {
        triggerOff();
      }
    }
  }

  void drawAllNotesOffButton() {
    std::string buttonName = "All notes off##paramGUI";
    if (ImGui::Button(buttonName.c_str(), ImVec2(ImGui::GetWindowWidth(), 0))) {
      synth().allNotesOff();
    }
  }

  /**
   * @brief Trigger a free voice. If no id provided the internal voice is
   * triggered
   * @param id
   */
  void triggerOn(int id = INT_MIN) {
    if (id == INT_MIN) {
      if (!mCurrentTriggerState) {
        mTriggerVoiceId =
            mSequencer.synth().triggerOn(&mControlVoice, 0, INT_MIN);
        mCurrentTriggerState = true;
      }
    } else {
      VoiceType *voice = synth().template getVoice<VoiceType>();
      configureVoiceFromGui(voice);
      synth().triggerOn(voice, 0, id);
    }
  }

  void triggerOff(int id = INT_MIN) {
    if (id == INT_MIN) {
      if (mCurrentTriggerState) {
        mControlVoice.free();
        while (mControlVoice.id() != -1) {
          // Wait a bit
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        mSequencer.synth().popFreeVoice(&mControlVoice);
        mCurrentTriggerState = false;
      }
    } else {
      synth().triggerOff(id);
    }
  }

  bool triggerButtonState() { return mCurrentTriggerState; }

  VoiceType *voice() { return &mControlVoice; }

  void recallPreset(int index) { mPresetHandler.recallPreset(index); }

  PresetSequencer &presetSequencer() { return mPresetSequencer; }
  PresetHandler &presetHandler() { return mPresetHandler; }

  PolySynth &synth() { return mSequencer.synth(); }
  SynthSequencer &synthSequencer() { return mSequencer; }
  SynthRecorder &synthRecorder() { return mRecorder; }

private:
  std::string mName;
  VoiceType mControlVoice;

  PresetHandler mPresetHandler;
  PresetSequencer mPresetSequencer;
  SequenceRecorder mPresetSequenceRecorder;

  //    PolySynth *mSynth;
  SynthSequencer mSequencer{TimeMasterMode::TIME_MASTER_AUDIO};
  SynthRecorder mRecorder;

  std::vector<std::shared_ptr<ParameterBundle>> mBundles;
  BundleGUIManager mBundleGui;

  bool mCurrentTriggerState{false};
  int mTriggerVoiceId{INT_MIN};

  int mCurrentTab = 1;
};

} // namespace al

#endif // AL_PARAMETERGUI_H
