#ifndef AL_ControlGUI_HPP
#define AL_ControlGUI_HPP

/*	Allocore --
        Multimedia / virtual environment application class library

        Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012-2018. The Regents of the University of California.
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
    ControlGUI class for simple GUI for Synth classes

        File author(s):
        Andrés Cabrera mantaraya36@gmail.com
*/

#include <map>
#include <string>

#include "al/io/al_ControlNav.hpp"
#include "al/io/al_Imgui.hpp"
#include "al/scene/al_DynamicScene.hpp"
#include "al/scene/al_SynthRecorder.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterBundle.hpp"
#include "al/ui/al_ParameterGUI.hpp"
#include "al/ui/al_ParameterMIDI.hpp"
#include "al/ui/al_PresetHandler.hpp"
#include "al/ui/al_PresetSequencer.hpp"

namespace al {

class GUIMarker {
public:
  enum class MarkerType { GROUP_BEGIN, GROUP_END, SEPARATOR };
  GUIMarker(MarkerType type, const char *name = nullptr) {
    mMarkerType = type;
    mName = name;
  }

  MarkerType getType() { return mMarkerType; }
  const char *getName() { return mName; }

private:
  MarkerType mMarkerType;
  const char *mName;
};

/**
 * @brief The ControlGUI class
 *
 * You must call init() before any draw calls.
 * @ingroup UI
 */
class ControlGUI {
  friend class GUIPanelDomain;

public:
  ControlGUI &registerParameterMeta(ParameterMeta &param);

  /// Register parameter using the streaming operator.
  ControlGUI &operator<<(ParameterMeta &newParam) {
    return registerParameterMeta(newParam);
  }
  ControlGUI &add(ParameterMeta &newParam) {
    return registerParameterMeta(newParam);
  }

  /// Register parameter using the streaming operator.
  ControlGUI &operator<<(ParameterMeta *newParam) {
    return registerParameterMeta(*newParam);
  }
  ControlGUI &add(ParameterMeta *newParam) {
    return registerParameterMeta(*newParam);
  }

  ControlGUI &registerParameterBundle(ParameterBundle &bundle);

  /// Register parameter using the streaming operator.
  ControlGUI &operator<<(ParameterBundle &newBundle) {
    return registerParameterBundle(newBundle);
  }
  ControlGUI &add(ParameterBundle &newBundle) {
    return registerParameterBundle(newBundle);
  }

  /// Register parameter using the streaming operator.
  ControlGUI &operator<<(ParameterBundle *newBundle) {
    return registerParameterBundle(*newBundle);
  }
  ControlGUI &add(ParameterBundle *newBundle) {
    return registerParameterBundle(*newBundle);
  }

  ControlGUI &registerNav(Nav &nav);

  /// Register nav using the streaming operator. A set of widgets are shown to
  /// control it.
  ControlGUI &operator<<(Nav &nav) { return registerNav(nav); }
  ControlGUI &add(Nav &nav) { return registerNav(nav); }

  ControlGUI &registerPresetHandler(PresetHandler &presetHandler,
                                    int numRows = -1, int numColumns = -1);

  /// Register preset handler using the streaming operator. GUI widgets for
  /// preset control are shown.
  ControlGUI &operator<<(PresetHandler &ph) {
    return registerPresetHandler(ph);
  }
  ControlGUI &add(PresetHandler &ph) { return registerPresetHandler(ph); }

  /// Register preset handler using the streaming operator. GUI widgets for
  /// preset control are shown.
  ControlGUI &operator<<(PresetHandler *ph) {
    return registerPresetHandler(*ph);
  }
  ControlGUI &add(PresetHandler *ph) { return registerPresetHandler(*ph); }

  ControlGUI &registerPresetSequencer(PresetSequencer &presetSequencer);

  /// Register preset sequencer using the streaming operator. GUI widgets for
  /// preset sequencing are shown.
  ControlGUI &operator<<(PresetSequencer &ps) {
    return registerPresetSequencer(ps);
  }
  ControlGUI &add(PresetSequencer &ps) { return registerPresetSequencer(ps); }

  ControlGUI &registerSequenceRecorder(SequenceRecorder &recorder);

  /// Register preset sequencer using the streaming operator. GUI widgets for
  /// preset sequencing are shown.
  ControlGUI &operator<<(SequenceRecorder &ps) {
    return registerSequenceRecorder(ps);
  }
  ControlGUI &add(SequenceRecorder &ps) { return registerSequenceRecorder(ps); }

  ControlGUI &registerSynthRecorder(SynthRecorder &recorder);

  /// Register a SynthRecorder. This will display GUI widgets to control it
  ControlGUI &operator<<(SynthRecorder &recorder) {
    return registerSynthRecorder(recorder);
  }
  ControlGUI &add(SynthRecorder &recorder) {
    return registerSynthRecorder(recorder);
  }

  ControlGUI &registerSynthSequencer(SynthSequencer &seq);

  /// Register a SynthSequencer. This will display GUI widgets to control it
  /// Will also register the PolySynth contained within it.
  ControlGUI &operator<<(SynthSequencer &seq) {
    return registerSynthSequencer(seq);
  }
  ControlGUI &add(SynthSequencer &seq) { return registerSynthSequencer(seq); }

  ControlGUI &registerDynamicScene(DynamicScene &scene);

  /// Register a SynthSequencer. This will display GUI widgets to control it
  /// Will also register the PolySynth contained within it.
  ControlGUI &operator<<(DynamicScene &scene) {
    return registerDynamicScene(scene);
  }
  ControlGUI &add(DynamicScene &scene) { return registerDynamicScene(scene); }

  ControlGUI &registerMarker(GUIMarker &marker);

  /// Register a SynthSequencer. This will display GUI widgets to control it
  /// Will also register the PolySynth contained within it.
  ControlGUI &operator<<(GUIMarker marker) { return registerMarker(marker); }
  ControlGUI &add(GUIMarker marker) { return registerMarker(marker); }

  void setTitle(std::string title) { mName = title; }

  void fixedPosition(bool fixed = true) { mFixed = fixed; }

  /**
   * @brief draws the GUI
   */
  void draw(Graphics &g);

  /**
   * @brief widget draw code without configuration
   */
  void drawWidgets();

  /**
   * @brief Call to set if this GUI manages ImGUI
   *
   * Set to false if you want to have additional ImGUI windows.
   * You will have to handle the initialization, cleanup and
   * begin/end calls yourself.
   */
  void manageImGUI(bool manage) { mManageIMGUI = manage; }

  /**
   * @brief initialize ImGUI.
   * @param x x position for the control window
   * @param y y position for the control window
   * @param manageImgui call imgui initialization and cleanup functions
   *
   * This function must be called before anu call to begin() or draw().
   * If you want to use mulitple ControlGUI objects, you need to set manageImgui
   * to false, and perform the ImGui initialization and cleanup manually
   */
  void init(int x = 5, int y = 5, bool manageImgui = true);

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
  bool usingInput() { return ParameterGUI::usingInput(); }

  /**
   * @brief get currently active index for a parameter bundle registerd with the
   * GUI
   * @param bundleName
   * @return current index
   *
   * Returns -1 if bundleName is not a registered bundle name
   */
  int getBundleCurrent(std::string bundleName) {
    if (mBundles.find(bundleName) != mBundles.end()) {
      return mBundles[bundleName]->currentBundle();
    }
    return -1;
  }

  /**
   * @brief Returns true if global controls is enabled for bundle
   * @param bundleName name of the bundle to query
   */
  bool getBundleIsGlobal(std::string bundleName) {
    if (mBundles.find(bundleName) != mBundles.end()) {
      return mBundles[bundleName]->bundleGlobal();
    }
    return false;
  }

  /**
   * @brief Set currently active bundle
   *
   * -1 enables global bundle control
   */
  void setCurrentBundle(std::string bundleName, int index) {
    mBundles[bundleName]->setCurrentBundle(index);
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

  static GUIMarker beginGroup(const char *groupName = nullptr) {
    return GUIMarker(GUIMarker::MarkerType::GROUP_BEGIN, groupName);
  }
  static GUIMarker endGroup() {
    return GUIMarker(GUIMarker::MarkerType::GROUP_END);
  }
  static GUIMarker separator() {
    return GUIMarker(GUIMarker::MarkerType::SEPARATOR);
  }

  std::function<void(void)> drawFunction = []() {};

protected:
private:
  // std::map<std::string, std::vector<Parameter *>> mParameters;
  // std::map<std::string, std::vector<ParameterBool *>> mParameterBools;
  std::map<std::string, std::vector<ParameterVec3 *>> mParameterVec3s;
  std::map<std::string, std::vector<ParameterVec4 *>> mParameterVec4s;

  std::map<std::string, std::vector<ParameterMeta *>> mElements;
  ParameterMeta *mLatestElement{nullptr};
  std::vector<ParameterMeta *>
      mGroupBeginAnchors; // refs to the parameters marking beginning and
                          // ending of groups
  std::vector<std::string> mGroupNames;
  std::vector<ParameterMeta *>
      mGroupEndAnchors; // refs to the parameters marking beginning and ending
                        // of groups
  std::vector<ParameterMeta *> mSeparatorAnchors;

  //  ParameterGUI mParameterGUI;

  PresetHandler *mPresetHandler{nullptr};
  int mPresetColumns, mPresetRows;
  PresetSequencer *mPresetSequencer{nullptr};
  SequenceRecorder *mSequenceRecorder{nullptr};
  SynthRecorder *mSynthRecorder{nullptr};
  SynthSequencer *mSynthSequencer{nullptr};
  PolySynth *mPolySynth{nullptr};
  DynamicScene *mScene{nullptr};
  std::map<std::string, BundleGUIManager *> mBundles;
  Nav *mNav{nullptr};

  std::string mName;
  bool mFixed{true};
  int mX, mY;

  float mGUIBackgroundAlpha = 0.3f;

  bool mManageIMGUI{true};

  //  int mCurrentSequencerItem{0};
  int mCurrentPresetSequencerItem{0};
  //  bool mRecordButtonValue{false};
  char **mSequencerItems;

  std::map<std::string, int> mCurrentBundle;
  std::map<std::string, bool> mBundleGlobal;

  void begin();
  void end();
};

} // namespace al

#endif
