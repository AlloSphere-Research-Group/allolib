
#include "al/ui/al_PresetHandler.hpp"
#include <cstdint>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "al/io/al_File.hpp"
#include "al/ui/al_ParameterBundle.hpp"

using namespace al;

namespace {

double variantAsDouble(const VariantValue &v) {
  switch (v.type()) {
  case VariantType::VARIANT_FLOAT:
    return v.get<float>();
  case VariantType::VARIANT_DOUBLE:
    return v.get<double>();
  case VariantType::VARIANT_INT32:
    return static_cast<double>(v.get<int32_t>());
  case VariantType::VARIANT_INT8:
    return static_cast<double>(v.get<int8_t>());
  case VariantType::VARIANT_INT16:
    return static_cast<double>(v.get<int16_t>());
  case VariantType::VARIANT_UINT8:
    return static_cast<double>(v.get<uint8_t>());
  case VariantType::VARIANT_UINT16:
    return static_cast<double>(v.get<uint16_t>());
  case VariantType::VARIANT_UINT32:
    return static_cast<double>(v.get<uint32_t>());
  default:
    return 0.0;
  }
}

bool isNumericVariant(VariantType t) {
  return t == VariantType::VARIANT_FLOAT || t == VariantType::VARIANT_DOUBLE ||
         t == VariantType::VARIANT_INT32 || t == VariantType::VARIANT_INT8 ||
         t == VariantType::VARIANT_INT16 || t == VariantType::VARIANT_UINT8 ||
         t == VariantType::VARIANT_UINT16 || t == VariantType::VARIANT_UINT32;
}

/// Coerce \p incoming to match \p current's type (float/int32 swaps used by
/// presets). Returns false if types are incompatible.
bool coerceFieldToMatch(const VariantValue &current, VariantValue &incoming) {
  if (current.type() == incoming.type()) {
    return true;
  }
  if (current.type() == VariantType::VARIANT_FLOAT &&
      incoming.type() == VariantType::VARIANT_INT32) {
    incoming = VariantValue(float(incoming.get<int32_t>()));
    return true;
  }
  if (current.type() == VariantType::VARIANT_INT32 &&
      incoming.type() == VariantType::VARIANT_FLOAT) {
    incoming = VariantValue(int32_t(incoming.get<float>()));
    return true;
  }
  if (current.type() == VariantType::VARIANT_FLOAT &&
      incoming.type() == VariantType::VARIANT_DOUBLE) {
    incoming = VariantValue(float(incoming.get<double>()));
    return true;
  }
  if (current.type() == VariantType::VARIANT_DOUBLE &&
      incoming.type() == VariantType::VARIANT_FLOAT) {
    incoming = VariantValue(double(incoming.get<float>()));
    return true;
  }
  if (current.type() == VariantType::VARIANT_DOUBLE &&
      incoming.type() == VariantType::VARIANT_INT32) {
    incoming = VariantValue(double(incoming.get<int32_t>()));
    return true;
  }
  if (current.type() == VariantType::VARIANT_INT32 &&
      incoming.type() == VariantType::VARIANT_DOUBLE) {
    incoming = VariantValue(int32_t(incoming.get<double>()));
    return true;
  }
  return false;
}

VariantValue lerpFields(const VariantValue &start, const VariantValue &end,
                        double factor) {
  if (start.type() == VariantType::VARIANT_STRING) {
    return end;
  }
  if (!isNumericVariant(start.type()) || !isNumericVariant(end.type())) {
    return end;
  }
  double v = variantAsDouble(start) +
             factor * (variantAsDouble(end) - variantAsDouble(start));
  if (start.type() == VariantType::VARIANT_INT32) {
    return VariantValue(static_cast<int32_t>(v));
  }
  if (start.type() == VariantType::VARIANT_DOUBLE) {
    return VariantValue(v);
  }
  return VariantValue(static_cast<float>(v));
}

VariantValue applyDeltaField(const VariantValue &start,
                             const VariantValue &delta, double factor) {
  if (start.type() == VariantType::VARIANT_STRING) {
    return factor >= 1.0 ? delta : start;
  }
  if (!isNumericVariant(start.type())) {
    return start;
  }
  double v = variantAsDouble(start) + factor * variantAsDouble(delta);
  if (start.type() == VariantType::VARIANT_INT32) {
    return VariantValue(static_cast<int32_t>(v));
  }
  if (start.type() == VariantType::VARIANT_DOUBLE) {
    return VariantValue(v);
  }
  return VariantValue(static_cast<float>(v));
}

std::string nestedBundlePrefix(const std::string &parentPrefix,
                               ParameterBundle *sub, const std::string &id,
                               size_t indexInId, size_t countForId) {
  std::string subPrefix = parentPrefix + "/" + sub->name() + "/" + id;
  if (countForId > 1) {
    subPrefix += "/" + std::to_string(indexInId);
  }
  return subPrefix;
}

void collectBundleParameters(ParameterBundle *bundle, const std::string &prefix,
                             PresetHandler::ParameterStates &values,
                             const std::vector<std::string> &skip) {
  for (ParameterMeta *p : bundle->parameters()) {
    if (std::find(skip.begin(), skip.end(), p->getFullAddress()) !=
        skip.end()) {
      continue;
    }
    std::vector<VariantValue> fields;
    p->getFields(fields);
    values[prefix + p->getFullAddress()] = std::move(fields);
  }
  for (const auto &subGroup : bundle->bundles()) {
    for (size_t j = 0; j < subGroup.second.size(); ++j) {
      ParameterBundle *sub = subGroup.second[j];
      collectBundleParameters(
          sub,
          nestedBundlePrefix(prefix, sub, subGroup.first, j,
                             subGroup.second.size()),
          values, skip);
    }
  }
}

void applyBundleParameters(ParameterBundle *bundle, const std::string &prefix,
                           PresetHandler::ParameterStates &targetValues) {
  for (ParameterMeta *p : bundle->parameters()) {
    auto it = targetValues.find(prefix + p->getFullAddress());
    if (it == targetValues.end()) {
      continue;
    }
    auto &fields = it->second;
    std::vector<VariantValue> current;
    p->getFields(current);
    if (current.size() == fields.size()) {
      for (size_t i = 0; i < current.size(); ++i) {
        coerceFieldToMatch(current[i], fields[i]);
      }
    }
    p->setFields(fields);
  }
  for (const auto &subGroup : bundle->bundles()) {
    for (size_t j = 0; j < subGroup.second.size(); ++j) {
      ParameterBundle *sub = subGroup.second[j];
      applyBundleParameters(sub,
                            nestedBundlePrefix(prefix, sub, subGroup.first, j,
                                               subGroup.second.size()),
                            targetValues);
    }
  }
}

bool applyFieldsAtBundleAddress(ParameterBundle *bundle,
                                const std::string &prefix,
                                const std::string &fullAddress,
                                std::vector<VariantValue> &fields) {
  for (ParameterMeta *p : bundle->parameters()) {
    if (prefix + p->getFullAddress() == fullAddress && fields.size() > 0) {
      p->setFields(fields);
      return true;
    }
  }
  for (const auto &subGroup : bundle->bundles()) {
    for (size_t j = 0; j < subGroup.second.size(); ++j) {
      ParameterBundle *sub = subGroup.second[j];
      if (applyFieldsAtBundleAddress(
              sub,
              nestedBundlePrefix(prefix, sub, subGroup.first, j,
                                 subGroup.second.size()),
              fullAddress, fields)) {
        return true;
      }
    }
  }
  return false;
}

void collectBundleMorphDeltas(ParameterBundle *bundle,
                              const std::string &prefix,
                              PresetHandler::ParameterStates &parameterStates,
                              PresetHandler::ParameterStates &startValues,
                              PresetHandler::ParameterStates &deltaValues,
                              const std::vector<std::string> &skip) {
  for (ParameterMeta *param : bundle->parameters()) {
    if (std::find(skip.begin(), skip.end(), param->getFullAddress()) !=
        skip.end()) {
      continue;
    }
    std::string address = prefix + param->getFullAddress();
    if (parameterStates.find(address) == parameterStates.end()) {
      continue;
    }
    startValues[address].clear();
    param->getFields(startValues[address]);
    auto &params = startValues[address];
    auto &targetValues = parameterStates[address];
    if (targetValues.size() < params.size()) {
      auto copyStart = params.begin();
      std::advance(copyStart, targetValues.size());
      targetValues.insert(targetValues.end(), copyStart, params.end());
    }
    auto &deltas = deltaValues[address];
    deltas.resize(targetValues.size());
    for (size_t i = 0; i < targetValues.size(); i++) {
      if (params[i].type() == VariantType::VARIANT_STRING ||
          !isNumericVariant(params[i].type()) ||
          !isNumericVariant(targetValues[i].type())) {
        deltas[i] = VariantValue(0.0f);
        continue;
      }
      double delta =
          variantAsDouble(targetValues[i]) - variantAsDouble(params[i]);
      if (params[i].type() == VariantType::VARIANT_INT32) {
        deltas[i] = VariantValue(static_cast<int32_t>(delta));
      } else if (params[i].type() == VariantType::VARIANT_DOUBLE) {
        deltas[i] = VariantValue(delta);
      } else {
        deltas[i] = VariantValue(static_cast<float>(delta));
      }
    }
  }
  for (const auto &subGroup : bundle->bundles()) {
    for (size_t j = 0; j < subGroup.second.size(); ++j) {
      ParameterBundle *sub = subGroup.second[j];
      collectBundleMorphDeltas(
          sub,
          nestedBundlePrefix(prefix, sub, subGroup.first, j,
                             subGroup.second.size()),
          parameterStates, startValues, deltaValues, skip);
    }
  }
}

std::string quotePresetString(const std::string &s) {
  std::string out = "\"";
  for (char c : s) {
    if (c == '\\' || c == '"') {
      out += '\\';
    }
    out += c;
  }
  out += '"';
  return out;
}

bool readPresetStringToken(std::istream &ss, std::string &out) {
  ss >> std::ws;
  int ch = ss.peek();
  if (ch == '"') {
    ss.get();
    out.clear();
    bool escape = false;
    while (ss.good()) {
      char c;
      ss.get(c);
      if (!ss.good()) {
        return false;
      }
      if (escape) {
        out += c;
        escape = false;
      } else if (c == '\\') {
        escape = true;
      } else if (c == '"') {
        return true;
      } else {
        out += c;
      }
    }
    return false;
  }
  return static_cast<bool>(std::getline(ss, out, ' '));
}

} // namespace

static void setBundleGroupValues(std::string fullAddress,
                                 std::vector<VariantValue> &values,
                                 std::vector<ParameterBundle *> bundles,
                                 std::string prefix) {
  for (size_t i = 0; i < bundles.size(); ++i) {
    if (applyFieldsAtBundleAddress(bundles[i], prefix + std::to_string(i),
                                   fullAddress, values)) {
      return;
    }
  }
}

// PresetHandler --------------------------------------------------------------

PresetHandler::PresetHandler(std::string rootDirectory, bool verbose)
    : PresetHandler(TimeMasterMode::TIME_MASTER_CPU, rootDirectory, verbose) {}

PresetHandler::PresetHandler(TimeMasterMode timeMasterMode,
                             std::string rootDirectory, bool verbose)
    : mVerbose(verbose), mRootDir(rootDirectory),
      mTimeMasterMode(timeMasterMode) {
  // setCurrentPresetMap("default");
  setRootPath(rootDirectory);
  if (mTimeMasterMode == TimeMasterMode::TIME_MASTER_CPU) {
    startCpuThread();
  }

  if (mTimeMasterMode == TimeMasterMode::TIME_MASTER_GRAPHICS ||
      mTimeMasterMode == TimeMasterMode::TIME_MASTER_AUDIO) {
    std::cerr << "ERROR: PresetSequencer: TimeMasterMode not supported, "
                 "treating as TIME_MASTER_CPU"
              << std::endl;
    startCpuThread();
  }
}

PresetHandler::~PresetHandler() { stopCpuThread(); }

void PresetHandler::setSubDirectory(std::string directory) {
  std::string path = getRootPath();
  if (!File::exists(path + directory)) {
    if (!Dir::make(path + directory)) {
      std::cout << "Error creating directory: " << path + directory
                << std::endl;
      return;
    }
  }
  setCurrentPresetMap();
  mSubDir = directory;
}

void PresetHandler::registerPresetCallback(
    std::function<void(int index, void *presetHandler, void *userData)> cb,
    void *userData) {
  mCallbacks.push_back(cb);
  mCallbackUdata.push_back(userData);
}

void PresetHandler::registerStoreCallback(
    std::function<void(int, std::string, void *)> cb, void *userData) {
  mStoreCallbacks.push_back(cb);
  mStoreCallbackUdata.push_back(userData);
}

void PresetHandler::registerMorphTimeCallback(
    Parameter::ParameterChangeCallback cb) {
  mMorphTime.registerChangeCallback(cb);
}

void PresetHandler::registerPresetMapCallback(PresetMapCallback cb) {
  mPresetsMapCbs.push_back(cb);
}

std::string PresetHandler::buildMapPath(std::string mapName,
                                        bool useSubDirectory) {
  std::string currentPath = File::conformDirectory(getRootPath());
  if (useSubDirectory) {
    currentPath += File::conformDirectory(mSubDir);
  }
  if (!(mapName.size() > 4 && mapName.substr(mapName.size() - 4) == ".txt") &&
      !(mapName.size() > 10 &&
        mapName.substr(mapName.size() - 10) == ".presetMap") &&
      !(mapName.size() > 18 &&
        mapName.substr(mapName.size() - 18) == ".presetMap_archive")) {
    mapName = mapName + ".presetMap";
  }

  return currentPath + mapName;
}

std::vector<std::string> PresetHandler::availablePresetMaps() {
  std::vector<std::string> mapList;
  std::string path = getCurrentPath();

  // get list of files ending in ".presetMap"
  static const std::string extension = ".presetMap";
  FileList sequence_files = filterInDir(path, [](const FilePath &f) {
    if (al::checkExtension(f, extension))
      return true;
    else
      return false;
  });

  // store found preset files
  for (int i = 0; i < sequence_files.count(); i += 1) {
    const FilePath &path = sequence_files[i];
    const std::string &name = path.file();
    // exclude extension when adding to sequence list
    std::string entryName = name.substr(0, name.size() - extension.size());
    if (std::find(mapList.begin(), mapList.end(), entryName) == mapList.end()) {
      mapList.push_back(entryName);
    }
  }

  std::sort(mapList.begin(), mapList.end(),
            [](const auto &lhs, const auto &rhs) {
              const auto result =
                  mismatch(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(),
                           [](const auto &lhs, const auto &rhs) {
                             return tolower(lhs) == tolower(rhs);
                           });

              return result.second != rhs.cend() &&
                     (result.first == lhs.cend() ||
                      tolower(*result.first) < tolower(*result.second));
            });

  return mapList;
}

void PresetHandler::storePreset(std::string name) {
  int index = -1;
  for (const auto &preset : mPresetsMap) {
    if (preset.second == name) {
      index = preset.first;
      break;
    }
  }
  if (index < 0) {
    for (const auto &preset : mPresetsMap) {
      if (index <= preset.first) {
        index = preset.first + 1;
      }
    } // FIXME this should look for the first "empty" preset.
  }
  storePreset(index, name);
}

void PresetHandler::storePreset(int index, std::string name, bool overwrite) {
  mFileLock.lock();
  // ':' causes issues with the text format for saving, so replace
  std::replace(name.begin(), name.end(), ':', '_');

  if (name == "") {
    for (const auto &preset : mPresetsMap) {
      if (preset.first == index) {
        name = preset.second;
        break;
      }
    }
    if (name == "") {
      name = "default";
    }
  }
  ParameterStates values;
  for (ParameterMeta *p : mParameters) {
    std::string address = p->getFullAddress();
    if (std::find(mSkipParameters.begin(), mSkipParameters.end(), address) ==
        mSkipParameters.end()) {
      std::vector<VariantValue> fields;
      p->getFields(fields);
      values[address] = fields;
    }
  }
  for (const auto &bundleGroup : mBundles) {
    std::string bundleName = "/" + bundleGroup.first + "/";
    for (size_t i = 0; i < bundleGroup.second.size(); ++i) {
      collectBundleParameters(bundleGroup.second[i],
                              bundleName + std::to_string(i), values,
                              mSkipParameters);
    }
  }

  savePresetValues(values, name, overwrite);
  mPresetsMap[index] = name;
  storeCurrentPresetMap();
  mCurrentPresetName = name;
  mFileLock.unlock();

  if (mUseCallbacks) {
    for (size_t i = 0; i < mStoreCallbacks.size(); ++i) {
      if (mStoreCallbacks[i]) {
        mStoreCallbacks[i](index, name, mStoreCallbackUdata[i]);
      }
    }
  }
}

void PresetHandler::recallPreset(std::string name) {
  morphTo(name, mMorphTime.get());

  int index = -1;
  for (const auto &preset : mPresetsMap) {
    if (preset.second == name) {
      index = preset.first;
      break;
    }
  }
  mCurrentPresetName = name;
  if (mUseCallbacks) {
    for (size_t i = 0; i < mCallbacks.size(); ++i) {
      if (mCallbacks[i]) {
        mCallbacks[i](index, this, mCallbackUdata[i]);
      }
    }
  }
}

void PresetHandler::setInterpolatedPreset(std::string presetName1,
                                          std::string presetName2,
                                          double factor) {
  ParameterStates values1 = loadPresetValues(presetName1);
  ParameterStates values2 = loadPresetValues(presetName2);
  setInterpolatedValues(values1, values2, factor);
}

void PresetHandler::setInterpolatedPreset(int index1, int index2,
                                          double factor) {
  auto presetNameIt1 = mPresetsMap.find(index1);
  auto presetNameIt2 = mPresetsMap.find(index2);
  if (presetNameIt1 != mPresetsMap.end() &&
      presetNameIt2 != mPresetsMap.end()) {
    setInterpolatedPreset(presetNameIt1->second, presetNameIt2->second, factor);
  } else {
    std::cout << "Invalid indeces for preset interpolation: " << index1 << ","
              << index2 << std::endl;
  }
}

void PresetHandler::morphTo(ParameterStates &parameterStates, float morphTime) {
  {
    std::lock_guard<std::mutex> lk(mTargetLock);
    //    mDeltaValues = parameterStates;
    mDeltaValues.clear();
    mStartValues.clear();
    for (ParameterMeta *param : mParameters) {
      auto address = param->getFullAddress();
      if (parameterStates.find(address) != parameterStates.end()) {
        mStartValues[address].clear();
        param->getFields(mStartValues[address]);
        auto &params = mStartValues[address];
        auto &targetValues = parameterStates[address];
        if (targetValues.size() < mStartValues[address].size()) {
          auto copyStart = mStartValues[address].begin();
          std::advance(copyStart, targetValues.size());
          targetValues.insert(targetValues.end(), copyStart,
                              mStartValues[address].end());
        } else if (targetValues.size() > mStartValues[address].size()) {
          std::cout << "morphTo() too many values. Discarding values"
                    << std::endl;
        }
        auto &deltaValues = mDeltaValues[address];
        deltaValues.resize(targetValues.size());
        for (size_t i = 0; i < targetValues.size(); i++) {
          if (params[i].type() == VariantType::VARIANT_STRING) {
            // Strings jump to target (no continuous morph).
            deltaValues[i] = VariantValue(0.0f);
            continue;
          }
          if (!isNumericVariant(params[i].type()) ||
              !isNumericVariant(targetValues[i].type())) {
            std::cout << "Parameter type unsupported in morph" << std::endl;
            deltaValues[i] = VariantValue(0.0f);
            continue;
          }
          double delta =
              variantAsDouble(targetValues[i]) - variantAsDouble(params[i]);
          if (params[i].type() == VariantType::VARIANT_INT32) {
            deltaValues[i] = VariantValue(static_cast<int32_t>(delta));
          } else if (params[i].type() == VariantType::VARIANT_DOUBLE) {
            deltaValues[i] = VariantValue(delta);
          } else {
            deltaValues[i] = VariantValue(static_cast<float>(delta));
          }
        }
        //        break;
      }
    }

    for (const auto &bundleGroup : mBundles) {
      std::string prefix = "/" + bundleGroup.first + "/";
      for (size_t i = 0; i < bundleGroup.second.size(); ++i) {
        collectBundleMorphDeltas(bundleGroup.second[i],
                                 prefix + std::to_string(i), parameterStates,
                                 mStartValues, mDeltaValues, mSkipParameters);
      }
    }

    if (morphTime != mMorphTime) {
      mMorphTime.set(morphTime);
    }
    mMorphStepCount = 0;
    if (mMorphTime.get() <= 0.0) {
      mTotalSteps.store(1);
    } else {
      mTotalSteps.store(ceilf(mMorphTime.get() / mMorphInterval));
    }
  }
  if (mVerbose) {
    std::cout << "start morph. steps " << mTotalSteps.load()
              << " time: " << mMorphTime.get() << " interval " << mMorphInterval
              << std::endl;
  }

  mCurrentPresetName = "";
}

void PresetHandler::morphTo(const std::string &presetName, float morphTime) {
  auto parameterStates = loadPresetValues(presetName);
  if (mUseCallbacks) {
    int index = -1;
    for (const auto &mapped : mPresetsMap) {
      if (mapped.second == presetName) {
        index = mapped.first;
        break;
      }
    }
    for (size_t i = 0; i < mCallbacks.size(); ++i) {
      if (mCallbacks[i]) {
        mCallbacks[i](index, this, mCallbackUdata[i]);
      }
    }
  }
  morphTo(parameterStates, morphTime);
}

std::string PresetHandler::recallPreset(int index) {
  auto presetNameIt = mPresetsMap.find(index);
  if (presetNameIt != mPresetsMap.end()) {
    recallPreset(presetNameIt->second);
    return presetNameIt->second;
  } else {
    std::cout << "No preset index " << index << std::endl;
  }
  return "";
}

void PresetHandler::recallPresetSynchronous(std::string name) {
  {
    mTotalSteps = 0;
    mMorphStepCount = 0;
    std::lock_guard<std::mutex> lk(mTargetLock);
    auto targetValues = loadPresetValues(name);
    for (ParameterMeta *param : mParameters) {
      std::vector<VariantValue> currentFields;
      param->getFields(currentFields);
      if (targetValues.find(param->getFullAddress()) != targetValues.end()) {
        auto &targetFields = targetValues[param->getFullAddress()];
        if (targetFields.size() == currentFields.size()) {
          for (size_t i = 0; i < currentFields.size(); i++) {
            if (!coerceFieldToMatch(currentFields[i], targetFields[i]) &&
                verbose()) {
              std::cerr << "Warning: type mismatch for "
                        << param->getFullAddress() << std::endl;
            }
          }
        }
        param->setFields(targetFields);
      } else {
        if (verbose()) {
          std::cerr << "Warning: parameter " << param->getFullAddress()
                    << " not matched" << std::endl;
        }
      }
    }
    for (const auto &bundleGroup : mBundles) {
      std::string bundleName = "/" + bundleGroup.first + "/";
      for (size_t i = 0; i < bundleGroup.second.size(); ++i) {
        applyBundleParameters(bundleGroup.second[i],
                              bundleName + std::to_string(i), targetValues);
      }
    }
  }
  int index = -1;
  for (const auto &preset : mPresetsMap) {
    if (preset.second == name) {
      index = preset.first;
      break;
    }
  }
  mCurrentPresetName = name;
  if (mUseCallbacks) {
    for (size_t i = 0; i < mCallbacks.size(); ++i) {
      if (mCallbacks[i]) {
        mCallbacks[i](index, this, mCallbackUdata[i]);
      }
    }
  }
}

std::string PresetHandler::recallPresetSynchronous(int index) {
  auto presetNameIt = mPresetsMap.find(index);
  if (presetNameIt != mPresetsMap.end()) {
    recallPresetSynchronous(presetNameIt->second);
    return presetNameIt->second;
  } else {
    std::cout << "No preset index " << index << std::endl;
  }
  return "";
}

std::map<int, std::string> PresetHandler::availablePresets() {
  return mPresetsMap;
}

std::string PresetHandler::getPresetName(int index) {
  if (mPresetsMap.find(index) != mPresetsMap.end()) {
    return mPresetsMap[index];
  } else {
    return std::string();
  }
}

void PresetHandler::skipParameter(std::string parameterAddr, bool skip) {
  std::unique_lock<std::mutex> lk(mSkipParametersLock);
  if (skip) {
    if (std::find(mSkipParameters.begin(), mSkipParameters.end(),
                  parameterAddr) == mSkipParameters.end()) {
      mSkipParameters.push_back(parameterAddr);
    }
  } else {
    auto position = std::find(mSkipParameters.begin(), mSkipParameters.end(),
                              parameterAddr);
    if (position != mSkipParameters.end()) {
      mSkipParameters.erase(position);
    }
  }
}

int PresetHandler::getCurrentPresetIndex() {
  std::map<int, std::string> presets = availablePresets();
  int current = -1;
  std::string currentPresetName = getCurrentPresetName();
  for (const auto &preset : presets) {
    if (preset.second == currentPresetName) {
      current = preset.first;
      break;
    }
  }
  return current;
}

float PresetHandler::getMorphTime() { return mMorphTime.get(); }

void PresetHandler::setMorphTime(float time) { mMorphTime.set(time); }

void PresetHandler::setMaxMorphTime(float time) { mMorphTime.max(time); }

void PresetHandler::stepMorphing(double stepTime) {
  double drift = mMorphInterval - stepTime;
  if (drift > 0.01) {
    std::cout << "Time drift = " << drift << std::endl;
  }
  stepMorphing();
}

std::string PresetHandler::getCurrentPath() {
  std::string relPath = getRootPath() + mSubDir;
  if (relPath.size() > 0) {
    relPath = File::conformPathToOS(relPath);
  }
  return relPath;
}

void PresetHandler::setRootPath(std::string path) {
  if (path.size() == 0) {
    mRootDir = "";
  } else if (!File::exists(path)) {
    if (!Dir::make(path)) {
      std::cerr << "Error creating directory: " << path << std::endl;
    } else {
      mRootDir = path;
    }
  } else {
    mRootDir = path;
  }
  setCurrentPresetMap();
}

std::string al::PresetHandler::getRootPath() {
  std::string relPath;
  if (mRootDir.size() > 0) {
    relPath = File::conformDirectory(mRootDir);
  }
  return relPath;
}

void PresetHandler::print() {
  std::cout << "Path: " << getCurrentPath() << std::endl;
  for (auto param : mParameters) {
    std::cout << param->getFullAddress() << std::endl;
  }
}

PresetHandler &PresetHandler::registerParameter(ParameterMeta &parameter) {
  mParameters.push_back(&parameter);
  return *this;
}

PresetHandler &PresetHandler::registerParameterBundle(ParameterBundle &bundle) {
  if (mBundles.find(bundle.name()) == mBundles.end()) {
    mBundles[bundle.name()] = std::vector<ParameterBundle *>();
  }
  mBundles[bundle.name()].push_back(&bundle);
  return *this;
}

std::map<int, std::string> PresetHandler::readPresetMap(std::string mapName) {
  std::map<int, std::string> presetsMap;
  std::string mapFullPath = buildMapPath(mapName, true);
  std::ifstream f(mapFullPath);
  if (!f.is_open()) {
    if (mVerbose) {
      std::cout << "Error while opening preset map file for reading: "
                << mapFullPath << std::endl;
    }
    return presetsMap;
  }
  std::string line;
  while (getline(f, line)) {
    if (line == "") {
      continue;
    }
    if (line.substr(0, 2) == "::") {
      if (mVerbose) {
        std::cout << "End preset map." << std::endl;
      }
      break;
    }
    std::stringstream ss(line);
    std::string index, name;
    std::getline(ss, index, ':');
    std::getline(ss, name, ':');
    presetsMap[std::stoi(index)] = name;
    //			std::cout << index << ":" << name << std::endl;
  }
  if (f.bad()) {
    if (mVerbose) {
      std::cout << "Error while opening preset map file for reading: "
                << mapFullPath << std::endl;
    }
  }
  return presetsMap;
}

void PresetHandler::setCurrentPresetMap(std::string mapName, bool autoCreate) {
  std::string mapFullPath = buildMapPath(mapName, true);
  if (autoCreate && !File::exists(mapFullPath) &&
      !File::isDirectory(mapFullPath)) {
    std::cout << "No preset map. Creating default." << std::endl;
    std::vector<std::string> presets;
    // FIXME put back cehcking of directory contents
    // Dir presetDir(getCurrentPath());
    // while(presetDir.read()) {
    // 	FileInfo info = presetDir.entry();
    // 	if (info.type() == FileInfo::REG) {
    // 		std::string name = info.name();
    // 		if (name.substr(name.size()-7) == ".preset") {
    // 			presets.push_back(info.name().substr(0,
    // name.size()-7));
    // 		}
    // 	}
    // }

    // get list of files ending in ".preset"
    FileList preset_files =
        filterInDir(getCurrentPath(), [](const FilePath &f) {
          if (al::checkExtension(f, ".preset"))
            return true;
          else
            return false;
        });

    // store found preset files
    for (int i = 0; i < preset_files.count(); i += 1) {
      const FilePath &path = preset_files[i];
      const std::string &name = path.file();
      presets.push_back(name.substr(0, name.size() - 7)); // exclude extension
    }

    for (size_t i = 0; i < presets.size(); ++i) {
      mPresetsMap[i] = presets[i];
    }
    mCurrentMapName = mapName;
    storeCurrentPresetMap(mapName, true);
  } else {
    mPresetsMap = readPresetMap(mapName);
    mCurrentMapName = mapName;
  }
  if (verbose()) {
    std::cout << "Setting preset map:" << mapName << std::endl;
  }
  for (const auto &cb : mPresetsMapCbs) {
    cb(mapName);
  }
}

void PresetHandler::changeParameterValue(std::string presetName,
                                         std::string parameterPath,
                                         float newValue) {
  ParameterStates parameters = loadPresetValues(presetName);
  for (auto &parameter : parameters) {
    if (parameter.first == parameterPath) {
      parameter.second[0] = newValue;
    }
  }
  savePresetValues(parameters, presetName, true);
}

int PresetHandler::asciiToPresetIndex(int ascii, int offset) {
  int index = -1;

  switch (ascii) {
  case '1':
    index = 0;
    break;
  case '2':
    index = 1;
    break;
  case '3':
    index = 2;
    break;
  case '4':
    index = 3;
    break;
  case '5':
    index = 4;
    break;
  case '6':
    index = 5;
    break;
  case '7':
    index = 6;
    break;
  case '8':
    index = 7;
    break;
  case '9':
    index = 8;
    break;
  case '0':
    index = 9;
    break;
  case 'q':
    index = 10;
    break;
  case 'w':
    index = 11;
    break;
  case 'e':
    index = 12;
    break;
  case 'r':
    index = 13;
    break;
  case 't':
    index = 14;
    break;
  case 'y':
    index = 15;
    break;
  case 'u':
    index = 16;
    break;
  case 'i':
    index = 17;
    break;
  case 'o':
    index = 18;
    break;
  case 'p':
    index = 19;
    break;
  case 'a':
    index = 20;
    break;
  case 's':
    index = 21;
    break;
  case 'd':
    index = 22;
    break;
  case 'f':
    index = 23;
    break;
  case 'g':
    index = 24;
    break;
  case 'h':
    index = 25;
    break;
  case 'j':
    index = 26;
    break;
  case 'k':
    index = 27;
    break;
  case 'l':
    index = 28;
    break;
  case ';':
    index = 29;
    break;
    ;
  case 'z':
    index = 30;
    break;
  case 'x':
    index = 31;
    break;
  case 'c':
    index = 32;
    break;
  case 'v':
    index = 33;
    break;
  case 'b':
    index = 34;
    break;
  case 'n':
    index = 35;
    break;
  case 'm':
    index = 36;
    break;
  case ',':
    index = 37;
    break;
  case '.':
    index = 38;
    break;
  case '/':
    index = 39;
    break;
  }
  if (index >= 0) {
    index += offset;
  }

  return index;
}

void PresetHandler::storeCurrentPresetMap(std::string mapName,
                                          bool useSubDirectory) {
  if (mapName.size() > 0) {
    mCurrentMapName = mapName;
  }
  std::string mapFullPath = buildMapPath(mCurrentMapName, useSubDirectory);
  std::ofstream f(mapFullPath);
  if (!f.is_open()) {
    if (mVerbose) {
      std::cout << "Error while opening preset map file: " << mapFullPath
                << std::endl;
    }
    return;
  }
  for (auto const &preset : mPresetsMap) {
    std::string line = std::to_string(preset.first) + ":" + preset.second;
    f << line << std::endl;
  }
  f << "::" << std::endl;
  if (f.bad()) {
    if (mVerbose) {
      std::cout << "Error while writing preset map file: " << mapFullPath
                << std::endl;
    }
  }
  f.close();
}

void PresetHandler::setInterpolatedValues(ParameterStates &startValues,
                                          ParameterStates &endValues,
                                          double factor) {
  for (auto &startValue : startValues) {
    std::vector<VariantValue> interpValues;
    interpValues.reserve(startValue.second.size());
    auto &endValue = endValues[startValue.first];
    assert(startValue.second.size() == endValue.size());
    if (factor != 1.0) {
      for (size_t i = 0; i < endValue.size(); i++) {
        if (!coerceFieldToMatch(startValue.second[i], endValue[i])) {
          // Also allow coercing start toward end when end is the stored type.
          if (!coerceFieldToMatch(endValue[i], startValue.second[i])) {
            std::cerr << "Parameter data type mismatch. Aborting." << std::endl;
            return;
          }
        }
        interpValues.push_back(
            lerpFields(startValue.second[i], endValue[i], factor));
      }
    } else {
      for (size_t i = 0; i < endValue.size(); i++) {
        coerceFieldToMatch(startValue.second[i], endValue[i]);
      }
      interpValues = endValue;
    }

    for (auto *param : mParameters) {
      if (param->getFullAddress() == startValue.first &&
          interpValues.size() > 0) {
        param->setFields(interpValues);
        break;
      }
    }

    for (const auto &bundleGroup : mBundles) {
      std::string prefix = "/" + bundleGroup.first + "/";
      setBundleGroupValues(startValue.first, interpValues, bundleGroup.second,
                           prefix);
    }
  }
}

void PresetHandler::setInterpolatedValuesDelta(ParameterStates &startValues,
                                               ParameterStates &deltaValues,
                                               double factor) {
  for (auto &startValue : startValues) {
    std::vector<VariantValue> interpValues;
    auto &deltaValue = deltaValues[startValue.first];
    interpValues.resize(deltaValue.size());
    assert(startValue.second.size() == deltaValue.size());
    for (size_t i = 0; i < deltaValue.size(); i++) {
      interpValues[i] =
          applyDeltaField(startValue.second[i], deltaValue[i], factor);
    }

    for (auto *param : mParameters) {
      if (param->getFullAddress() == startValue.first &&
          interpValues.size() > 0) {
        param->setFields(interpValues);
        break;
      }
    }

    for (const auto &bundleGroup : mBundles) {
      std::string prefix = "/" + bundleGroup.first + "/";
      setBundleGroupValues(startValue.first, interpValues, bundleGroup.second,
                           prefix);
    }
  }
}

bool PresetHandler::stepMorphing() {
  uint64_t totalSteps = mTotalSteps.load();
  uint64_t stepCount = mMorphStepCount.fetch_add(1);
  if (stepCount <= totalSteps && totalSteps > 0) {
    mMorphingActive.store(true);
    double morphPhase = double(stepCount) / totalSteps;
    if (totalSteps == 1) {
      morphPhase = 1.0;
    }
    std::lock_guard<std::mutex> lk(mTargetLock);
    setInterpolatedValuesDelta(mStartValues, mDeltaValues, morphPhase);
    return true;
  }
  mMorphingActive.store(false);
  return false;
}

void PresetHandler::morphingFunction(al::PresetHandler *handler) {
  while (handler->mCpuThreadRunning) {
    auto start = std::chrono::high_resolution_clock::now();
    handler->stepMorphing();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count() *
        1.0e-6;

    if (duration > handler->mMorphInterval) {
      std::cout << "WARNING missed morphing step time by "
                << duration - handler->mMorphInterval << " total=" << duration
                << std::endl;
    }
    std::this_thread::sleep_until(
        start +
        std::chrono::microseconds((long long)(handler->mMorphInterval * 1e6)));
  }
}

PresetHandler::ParameterStates
PresetHandler::getBundleStates(ParameterBundle *bundle, std::string id) {
  ParameterStates values;
  std::string bundlePrefix = bundle->name() + "/" + id;
  for (ParameterMeta *p : bundle->parameters()) {
    values[bundlePrefix + p->getFullAddress()] = std::vector<VariantValue>();
    p->getFields(values[bundlePrefix + p->getFullAddress()]);
  }
  for (const auto &b : bundle->bundles()) {
    for (auto *bundle : b.second) {
      auto subBundleValues = getBundleStates(bundle, b.first);
      for (const auto &bundleValue : subBundleValues) {
        values[bundlePrefix + "/" + bundleValue.first] = bundleValue.second;
      }
    }
  }
  return values;
}

PresetHandler::ParameterStates
PresetHandler::loadPresetValues(std::string name) {
  ParameterStates preset;
  std::lock_guard<std::mutex> lock(mFileLock); // Protect loading and saving
  std::lock_guard<std::mutex> lock2(mSkipParametersLock); // Protect skip list
  std::string path = getCurrentPath();
  if (path.back() != '/') {
    path += "/";
  }
  std::string line;
  std::ifstream f(path + name + ".preset");
  if (!f.is_open()) {
    if (mVerbose) {
      std::cout << "Error while opening preset file: "
                << path + name + ".preset" << std::endl;
    }
  }
  while (getline(f, line)) {
    if (line.substr(0, 2) == "::") {
      if (mVerbose) {
        std::cout << "Found preset : " << line << std::endl;
      }
      while (getline(f, line)) {
        if (line.size() < 2) {
          continue;
        }
        if (line.substr(0, 2) == "::") {
          if (mVerbose) {
            std::cout << "End preset." << std::endl;
          }
          break;
        }
        std::stringstream ss(line);
        std::string address, type;
        std::vector<VariantValue> values;
        std::getline(ss, address, ' ');
        std::getline(ss, type, ' ');
        auto currentType = type.begin();
        while (currentType != type.end() && ss.good()) {
          if (*currentType == 'f') {
            float fval = 0.f;
            if (!(ss >> fval)) {
              break;
            }
            values.push_back(fval);
          } else if (*currentType == 'i') {
            int ival = 0;
            if (!(ss >> ival)) {
              break;
            }
            values.push_back(ival);
          } else if (*currentType == 's') {
            std::string sval;
            if (!readPresetStringToken(ss, sval)) {
              break;
            }
            values.push_back(sval);
          } else {
            break;
          }
          ++currentType;
        }

        if (address.size() > 0 && address[0] != '#' && type.size() > 0 &&
            std::find(mSkipParameters.begin(), mSkipParameters.end(),
                      address) == mSkipParameters.end()) {
          preset[address] = values;
        }
      }
    }
  }
  if (f.bad()) {
    if (mVerbose) {
      std::cout << "Error while writing preset file: "
                << path + name + ".preset" << std::endl;
    }
  }
  f.close();
  return preset;
}

bool PresetHandler::savePresetValues(const ParameterStates &values,
                                     std::string presetName, bool overwrite) {
  bool ok = true;
  std::string path = getCurrentPath();
  std::string fileName = path + presetName + ".preset";
  std::ifstream infile(fileName);
  int number = 0;
  while (infile.good() && !overwrite) {
    fileName = path + presetName + "_" + std::to_string(number) + ".preset";
    infile.close();
    infile.open(fileName);
    number++;
  }
  infile.close();
  std::ofstream f(fileName);
  if (!f.is_open()) {
    if (mVerbose) {
      std::cout << "Error while opening preset file for write: " << fileName
                << std::endl;
    }
    return false;
  }
  f << "::" + presetName << std::endl;
  for (const auto &value : values) {
    std::string types, valueString;
    for (auto &value2 : value.second) {
      if (value2.type() == VariantType::VARIANT_FLOAT) {
        types += "f";
        valueString += std::to_string(value2.get<float>()) + " ";
      } else if (value2.type() == VariantType::VARIANT_DOUBLE) {
        types += "f";
        valueString +=
            std::to_string(static_cast<float>(value2.get<double>())) + " ";
      } else if (value2.type() == VariantType::VARIANT_STRING) {
        types += "s";
        valueString += quotePresetString(value2.get<std::string>()) + " ";
      } else if (value2.type() == VariantType::VARIANT_INT32) {
        types += "i";
        valueString += std::to_string(value2.get<int32_t>()) + " ";
      }
    }
    if (!valueString.empty() && valueString.back() == ' ') {
      valueString.pop_back();
    }
    f << value.first << " " << types << " " << valueString << std::endl;
  }
  f << "::" << std::endl;
  if (f.bad()) {
    if (mVerbose) {
      std::cout << "Error while writing preset file: " << fileName << std::endl;
    }
    ok = false;
  }
  f.close();
  return ok;
}

void PresetHandler::setTimeMaster(TimeMasterMode masterMode) {
  stopCpuThread();
  mTimeMasterMode = masterMode;
  if (masterMode == TimeMasterMode::TIME_MASTER_CPU) {
    mCpuThreadRunning = true;
    mMorphingThread =
        std::make_unique<std::thread>(PresetHandler::morphingFunction, this);
  }
  if (mTimeMasterMode == TimeMasterMode::TIME_MASTER_GRAPHICS ||
      mTimeMasterMode == TimeMasterMode::TIME_MASTER_AUDIO) {
    std::cerr << "ERROR: PresetSequencer: TimeMasterMode not supported, "
                 "treating as TIME_MASTER_CPU"
              << std::endl;
  }
}

void PresetHandler::startCpuThread() {
  mCpuThreadRunning = true;
  mMorphingThread =
      std::make_unique<std::thread>(PresetHandler::morphingFunction, this);
}

void PresetHandler::stopCpuThread() {
  mCpuThreadRunning = false;
  //  mMorphConditionVar.notify_all();
  // mMorphLock.lock();
  if (mMorphingThread) {
    mMorphingThread->join();
    mMorphingThread = nullptr;
  }
}
