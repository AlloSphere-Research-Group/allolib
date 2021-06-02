#ifndef AL_PRESETHANDLER_H
#define AL_PRESETHANDLER_H

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
   Preset classes that encapsulates storing values for groups of parameters
   File author(s):
   Andrés Cabrera mantaraya36@gmail.com
*/

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "al/protocol/al_OSC.hpp"
#include "al/system/al_Time.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterServer.hpp"

namespace al {

/**
 * @brief The PresetHandler class handles sorting and recalling of presets.
 * @ingroup UI
 *
 * Presets are saved by name with the ".preset" suffix.
 */
class PresetHandler {
public:
  typedef std::map<std::string, std::vector<VariantValue>> ParameterStates;
  /**
   * @brief PresetHandler contructor
   *
   * @param rootDirectory sets the root directory for preset and preset map
   * storage
   * @param verbose if true, print diagnostic messages
   *
   */
  PresetHandler(std::string rootDirectory = "presets", bool verbose = false);

  /**
   * @brief Constructor with option to set time master mode
   * @param timeMasterMode
   *
   * Only two modes are currently valid for PresetHandler:
   * TIME_MASTER_CPU and TIME_MASTER_ASYNC. The first will start a CPU
   * thread that handles morphing and setting values, the second does not
   * start the thread, so user must manually call tick()
   */
  PresetHandler(TimeMasterMode timeMasterMode,
                std::string rootDirectory = "presets", bool verbose = false);

  ~PresetHandler();

  /**
   * @brief Stores preset
   * @param name the name of the preset
   *
   * Since all presets are assinged an index, calling this function will give
   * the preset a free index and then call storePreset(index, name). If the
   * preset name already exists, it will overwrite the existing preset without
   * assigning a new index.
   */
  void storePreset(std::string name);

  /**
   * @brief Store preset at index. The name argument specifies the preset name
   *
   * @param index
   * @param name
   * @param overwrite if false, and preset file exists it is not overwritten and
   * a number is appended
   *
   * The preset name also determines the filename under which the preset is
   * saved, so it must be unique. If name is empty, a unique name is generated.
   * If name exists, a number is appended to the preset name. The link between
   * preset index and preset name is store within the preset map file that is
   * stored in the path for the PresetHandler, see getCurrentPath()
   *
   */
  void storePreset(int index, std::string name = "", bool overwrite = true);
  /**
   * @brief Recall a preset by name
   * @param name
   *
   * The preset should be a file on disk in the PresetHandler's root
   * directory and should have the ".preset" extension. See also
   * setSubDirectory().
   */
  void recallPreset(std::string name);

  /**
   * @brief Recall a preset by index number
   * @param index
   * @return the name of the preset corresponding to index. Empty if index not
   * valid.
   *
   * The preset map file (by default called default.presetMap)
   * is used to map preset names on disk to indeces. You can
   * set alternative preset map files using setCurrentPresetMap().
   * See also PresetMapper for handling and archiving preset maps.
   */
  std::string recallPreset(int index);

  void recallPresetSynchronous(std::string name);

  /**
   * @brief recall immediately (not using the morph thread)
   * @param index
   */
  std::string recallPresetSynchronous(int index);

  /**
   * @brief Set parameters to values interpolated between two presets
   * @param index1 index of the first preset
   * @param index2 index of the second preset
   * @param factor A value between 0-1 to determine interpolation
   * @param synchronous The values are set instantly and synchronous to this
   * call
   *
   * A factor of 0 uses preset 1 and a factor of 1 uses preset 2. Values
   * in between result in linear interpolation of the values.
   */
  void setInterpolatedPreset(int index1, int index2, double factor);

  void setInterpolatedPreset(std::string presetName1, std::string presetName2,
                             double factor);

  //  static void setParameterValues(ParameterMeta *param,
  //                                 std::vector<VariantValue> &values);
  /**
   * @brief Interpolate between start and end values according to
   * factor
   */
  void setInterpolatedValues(ParameterStates &startValues,
                             ParameterStates &endValues, double factor = 1.0);

  std::map<int, std::string> availablePresets();
  std::string getPresetName(int index);
  std::string getCurrentPresetName() { return mCurrentPresetName; }

  /**
   * @brief Add or remove a parameter address from group that will be skipped
   * when recalling presets
   * @param parameter address of parameter to skip
   * @param skip set to false if you want to remove from skip list
   *
   * This is used within loadPresetValues(), so it will affect both synchronous
   * and asynchronous recall.
   */
  void skipParameter(std::string parameterAddr, bool skip = true);

  int getCurrentPresetIndex();

  float getMorphTime();
  void setMorphTime(float time);
  void setMaxMorphTime(float time);
  void stopMorphing() { mTotalSteps.store(0); }
  void morphTo(ParameterStates &parameterStates, float morphTime);
  void morphTo(std::string presetName, float morphTime);

  void setMorphStepTime(float stepTime) { mMorphInterval = stepTime; }

  void stepMorphing(double stepTime);

  /// Step morphing to adjust parameter values to next step. You need to call
  /// this function only if TimeMasterMode is TIME_MASTER_ASYNC
  void stepMorphing();

  void setSubDirectory(std::string directory);
  std::string getSubDirectory() { return mSubDir; }

  /// Path including subdirectory if any
  std::string getCurrentPath();

  void setRootPath(std::string path);

  /// Base path without appending sub directory
  std::string getRootPath();

  /** Display information about preset handler, including path and
   * registered parameters.
   */
  void print();

  /**
   * @brief Register a callback to be notified when a preset is loaded
   * @param cb The callback function
   * @param userData data to be passed to the callback
   */
  void registerPresetCallback(
      std::function<void(int index, void *sender, void *userData)> cb,
      void *userData = nullptr);

  /**
   * @brief Register a callback to be notified when a preset is stored
   * @param cb The callback function
   * @param userData data to be passed to the callback
   */
  void registerStoreCallback(
      std::function<void(int index, std::string name, void *userData)> cb,
      void *userData = nullptr);

  /**
   * @brief Register a callback to be notified when morph time parameter is
   * changed
   * @param cb The callback function
   */
  void registerMorphTimeCallback(Parameter::ParameterChangeCallback cb);

  typedef const std::function<void(std::string)> PresetMapCallback;
  /**
   * @brief Register a callback to be notified when preset map cahges
   * @param cb The callback function
   */
  void registerPresetMapCallback(PresetMapCallback cb);

  PresetHandler &registerParameter(ParameterMeta &parameter);

  PresetHandler &operator<<(ParameterMeta &param) {
    return this->registerParameter(param);
  }

  PresetHandler &registerParameterBundle(ParameterBundle &bundle);

  PresetHandler &operator<<(ParameterBundle &bundle) {
    return this->registerParameterBundle(bundle);
  }

  std::vector<ParameterMeta *> parameters() { return mParameters; }

  std::string buildMapPath(std::string mapName, bool useSubDirectory = false);

  std::vector<std::string> availablePresetMaps();

  std::map<int, std::string> readPresetMap(std::string mapName = "default");

  void setCurrentPresetMap(std::string mapName = "default",
                           bool autoCreate = false);

  void setPresetMap(std::map<int, std::string> presetsMap) {
    mPresetsMap = presetsMap;
  }

  void storeCurrentPresetMap(std::string mapName = "",
                             bool useSubDirectory = false);

  /**
   * @brief useCallbacks determines whether to call the internal callbacks
   * @param use
   *
   * The callbacks set by registerStoreCallback() and registerPresetCallback()
   * are only called if this is set to true. The value is true by default.
   */
  void useCallbacks(bool use) { mUseCallbacks = use; }

  void changeParameterValue(std::string presetName, std::string parameterPath,
                            float newValue);

  /**
   * @brief Map QWERTY ascii keys to presets 0-49
   * @param ascii ascii code of the key to mapt
   * @param offset add an offset on output
   * @return the mapped value 0-39 plus offset. Returns -1 on failure to map.
   *
   * This maps four rows of 10 keys on the ASCII keyboard (regular QWERTY US
   * keys) to numbers 0-39. This can be useful for quick preset mapping using
   * the whole keyboard.
   */
  static int asciiToPresetIndex(int ascii, int offset = 0);

  [[deprecated]] void verbose(bool isVerbose) { mVerbose = isVerbose; }
  void setVerbose(bool isVerbose = true) { mVerbose = isVerbose; }
  bool verbose() { return mVerbose; }

  /**
   * @brief load preset into parameter states data structure without setting
   * values
   * @param name name of the preset to load
   * @return the state of the parameters in the loaded prese
   */
  ParameterStates loadPresetValues(std::string name);

  /**
   * @brief save list of parameter states into text preset file
   * @param values the values of parameters to store
   * @param presetName name of preset to store
   * @param overwrite true overwrites otherwise append unique number
   * @return true if no errors.
   */
  bool savePresetValues(const ParameterStates &values, std::string presetName,
                        bool overwrite = true);

  void setTimeMaster(TimeMasterMode masterMode);

  void startCpuThread();
  void stopCpuThread();

private:
  //  std::vector<float> getParameterValue(ParameterMeta *p);
  //  void setParametersInBundle(ParameterBundle *bundle, std::string
  //  bundlePrefix,
  //                             PresetHandler *handler, double factor = 1.0);
  static void morphingFunction(PresetHandler *handler);

  ParameterStates getBundleStates(ParameterBundle *bundle, std::string id);

  bool mVerbose{false};
  bool mUseCallbacks{true};
  std::string mRootDir;
  std::string mSubDir; // Optional sub directory, e.g. for preset map archives

  std::string mCurrentMapName;
  std::string mCurrentPresetName;
  std::vector<ParameterMeta *> mParameters;

  std::vector<std::string> mSkipParameters;
  std::mutex mSkipParametersLock;

  std::map<std::string, std::vector<ParameterBundle *>> mBundles;

  // Protects file writing from this class. Only one file may be written at
  // a time.
  std::mutex mFileLock;

  std::mutex mTargetLock;
  ParameterStates mTargetValues;
  ParameterStates mStartValues;

  TimeMasterMode mTimeMasterMode{TimeMasterMode::TIME_MASTER_CPU};

  Parameter mMorphTime{"morphTime", "", 0.0, 0.0, 20.0};

  std::atomic<uint64_t> mMorphStepCount{0};
  std::atomic<uint64_t> mTotalSteps{0};
  //  std::atomic<float> mCurrentMorphIndex;
  bool mCpuThreadRunning{false}; // To keep the morphing thread alive
  std::unique_ptr<std::thread> mMorphingThread;
  //  std::condition_variable mMorphConditionVar;
  double mMorphInterval{0.02};
  //  std::atomic<bool> mMorphing;

  std::vector<std::function<void(int index, void *sender, void *userData)>>
      mCallbacks;
  std::vector<void *> mCallbackUdata;
  std::vector<std::function<void(int index, std::string name, void *userData)>>
      mStoreCallbacks;
  std::vector<void *> mStoreCallbackUdata;
  std::vector<std::function<void(std::string)>> mPresetsMapCbs;

  std::map<int, std::string> mPresetsMap;
};

} // namespace al

#endif // AL_PRESETHANDLER_H
