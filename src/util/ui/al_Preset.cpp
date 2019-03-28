
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cassert>

#include "al/util/ui/al_Preset.hpp"
#include "al/core/io/al_File.hpp"

using namespace al;


// PresetHandler --------------------------------------------------------------

PresetHandler::PresetHandler(std::string rootDirectory, bool verbose) :
	mVerbose(verbose), mUseCallbacks(true), mRootDir(rootDirectory),
    mRunning(true), mMorphRemainingSteps(-1),
    mMorphInterval(0.05f), mMorphTime("morphTime", "", 0.0, "", 0.0, 20.0), mMorphingThread(PresetHandler::morphingFunction, this)
{
	if (!File::exists(rootDirectory)) {
		if (!Dir::make(rootDirectory)) {
			std::cout << "Error creating directory: " << rootDirectory << std::endl;
		}
	}
	setCurrentPresetMap("default");
}

PresetHandler::~PresetHandler()
{
	stopMorph();
	mRunning = false;
	mMorphConditionVar.notify_all();
	// mMorphLock.lock();
	mMorphingThread.join();
	// mMorphLock.unlock();
}

void PresetHandler::setSubDirectory(std::string directory)
{
	std::string path = getRootPath();
	if (!File::exists(path + directory)) {
		if (!Dir::make(path + directory)) {
			std::cout << "Error creating directory: " << mRootDir << std::endl;
			return;
		}
	}
	mSubDir = directory;
}

void PresetHandler::registerPresetCallback(std::function<void (int, void *, void *)> cb,
                                           void *userData)
{
	mCallbacks.push_back(cb);
	mCallbackUdata.push_back(userData);
}


void PresetHandler::registerStoreCallback(std::function<void (int, std::string, void *)> cb, void *userData)
{
	mStoreCallbacks.push_back(cb);
	mStoreCallbackUdata.push_back(userData);
}

void PresetHandler::registerMorphTimeCallback(Parameter::ParameterChangeCallback cb)
{
    mMorphTime.registerChangeCallback(cb);
}

void PresetHandler::registerPresetMapCallback(PresetMapCallback cb)
{
    mPresetsMapCbs.push_back(cb);
}

std::string PresetHandler::buildMapPath(std::string mapName, bool useSubDirectory)
{
	std::string currentPath = File::conformDirectory(getRootPath());
	if (useSubDirectory) {
		currentPath += File::conformDirectory(mSubDir);
	}
	if ( !(mapName.size() > 4 && mapName.substr(mapName.size() - 4) == ".txt")
	     && !(mapName.size() > 10 && mapName.substr(mapName.size() - 10) == ".presetMap")
	     && !(mapName.size() > 18 && mapName.substr(mapName.size() - 18) == ".presetMap_archive")) {
		mapName = mapName + ".presetMap";
	}

    return currentPath + mapName;
}

std::vector<std::string> PresetHandler::availablePresetMaps()
{
    std::vector<std::string> mapList;
    std::string path = getCurrentPath();

    // get list of files ending in ".presetMap"
    static const std::string extension = ".presetMap";
    FileList sequence_files = filterInDir(path, [](const FilePath& f){
        if (al::checkExtension(f, extension)) return true;
        else return false;
    });

    // store found preset files
    for (int i = 0; i < sequence_files.count(); i += 1) {
        const FilePath& path = sequence_files[i];
        const std::string& name = path.file();
        // exclude extension when adding to sequence list
        std::string entryName = name.substr(0, name.size()-extension.size());
        if (std::find(mapList.begin(), mapList.end(), entryName)== mapList.end()) {
            mapList.push_back(entryName);
        }
    }

    std::sort(mapList.begin(), mapList.end(), [](const auto& lhs, const auto& rhs){
        const auto result = mismatch(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), [](const auto& lhs, const auto& rhs){return tolower(lhs) == tolower(rhs);});

        return result.second != rhs.cend() && (result.first == lhs.cend() || tolower(*result.first) < tolower(*result.second));
    });

    return mapList;
}

void PresetHandler::storePreset(std::string name)
{
	int index = -1;
	for (auto preset: mPresetsMap) {
		if (preset.second == name) {
			index = preset.first;
			break;
		}
	}
	if (index < 0) {
		for (auto preset: mPresetsMap) {
			if (index <= preset.first) {
				index = preset.first + 1;
			}
		} // FIXME this should look for the first "empty" preset.
	}
	storePreset(index, name);
}

void PresetHandler::storePreset(int index, std::string name, bool overwrite)
{
	mFileLock.lock();
	// ':' causes issues with the text format for saving, so replace
	std::replace( name.begin(), name.end(), ':', '_');

	if (name == "") {
		for (auto preset: mPresetsMap) {
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
	for(ParameterMeta *p: mParameters) {
        values[p->getFullAddress()] = getParameterValue(p);
	}
    for (auto bundleGroup: mBundles) {
        std::string bundleName = "/" + bundleGroup.first + "/";

        for(unsigned int i = 0; i < bundleGroup.second.size(); i++) {
            std::string bundlePrefix = bundleName + std::to_string(i);
            for(ParameterMeta *p: bundleGroup.second.at(i)->parameters()) {
                values[bundlePrefix + p->getFullAddress()] = getParameterValue(p);
            }
            for(auto subBundle: bundleGroup.second.at(i)->bundles()) {
                auto bundleStates = getBundleStates(subBundle.second, subBundle.first);
                for (auto &bundleValues: bundleStates) {
                    values[bundlePrefix  + "/" + bundleValues.first] = bundleValues.second;

                }
            }
        }
    }

	savePresetValues(values, name, overwrite);
	mPresetsMap[index] = name;
	storeCurrentPresetMap();
    mCurrentPresetName = name;
	mFileLock.unlock();

	if (mUseCallbacks) {
		for(size_t i = 0; i < mStoreCallbacks.size(); ++i) {
			if (mStoreCallbacks[i]) {
				mStoreCallbacks[i](index, name, mStoreCallbackUdata[i]);
			}
		}
	}
}

void PresetHandler::recallPreset(std::string name)
{
	{
		if (mMorphRemainingSteps.load() >= 0) {
			mMorphRemainingSteps.store(-1);
			std::lock_guard<std::mutex> lk(mTargetLock);
		}
		mTargetValues = loadPresetValues(name);
		mMorphRemainingSteps.store(1.0f + ceilf(mMorphTime.get() / mMorphInterval));
	}
	mMorphConditionVar.notify_one();
	int index = -1;
	for (auto preset: mPresetsMap) {
		if (preset.second == name) {
			index = preset.first;
			break;
		}
	}
	mCurrentPresetName = name;
	if (mUseCallbacks) {
		for(size_t i = 0; i < mCallbacks.size(); ++i) {
			if (mCallbacks[i]) {
				mCallbacks[i](index, this, mCallbackUdata[i]);
			}
		}
	}
}

void PresetHandler::setInterpolatedPreset(std::string presetName1, std::string presetName2, double factor, bool synchronous)
{
    ParameterStates values1 = loadPresetValues(presetName1);
    ParameterStates values2 = loadPresetValues(presetName2);
    if (synchronous) {
        for(auto value: values1) {
            if (values2.count(value.first) > 0) { // if para std::cout << meter name match exists
                mTargetValues[value.first] = value.second;
                for (ParameterMeta *param: mParameters) {
                    if (param->getFullAddress() == value.first) {
                        std::vector<float> newValues;
                        for (unsigned int index = 0; index < value.second.size(); index++) {
                            newValues.push_back(value.second[index] + (values2[value.first][index] - value.second[index])* factor);
                        }
                        setParameterValues(param, newValues);
                    }
                }
            }
        }
    } else {
        if (mMorphRemainingSteps.load() >= 0) {
            mMorphRemainingSteps.store(-1);
            std::lock_guard<std::mutex> lk(mTargetLock); // Wait for morph function loop to process
        }
        mTargetValues.clear();
        for(auto value: values1) {
            if (values2.count(value.first) > 0) { // if para std::cout << meter name match exists
                mTargetValues[value.first] = value.second;
                for (unsigned int index = 0; index < value.second.size(); index++) {
                    mTargetValues[value.first][index] = value.second[index] + (values2[value.first][index] - value.second[index])* factor;
                }
            }
        }
    }
    mMorphConditionVar.notify_one();
}

void PresetHandler::setInterpolatedPreset(int index1, int index2, double factor, bool synchronous)
{
	auto presetNameIt1 = mPresetsMap.find(index1);
	auto presetNameIt2 = mPresetsMap.find(index2);
	if (presetNameIt1 != mPresetsMap.end()
	        && presetNameIt2 != mPresetsMap.end()) {
        setInterpolatedPreset(presetNameIt1->second, presetNameIt2->second, factor, synchronous);
	} else {
		std::cout << "Invalid indeces for preset interpolation: " << index1 << "," << index2 << std::endl;
	}
}

void PresetHandler::morphTo(ParameterStates &parameterStates, float morphTime)
{
	{
		if (mMorphRemainingSteps.load() >= 0) {
			mMorphRemainingSteps.store(-1);
			std::lock_guard<std::mutex> lk(mTargetLock);
		}
		mMorphTime.set(morphTime);
		mTargetValues = parameterStates;
		mMorphRemainingSteps.store(1 + ceil(mMorphTime.get() / mMorphInterval));
	}
	mMorphConditionVar.notify_one();
//	int index = -1;
//	for (auto preset: mPresetsMap) {
//		if (preset.second == name) {
//			index = preset.first;
//			break;
//		}
//	}
	mCurrentPresetName = "";
//	if (mUseCallbacks) {
//		for(int i = 0; i < mCallbacks.size(); ++i) {
//			if (mCallbacks[i]) {
//				mCallbacks[i](index, this, mCallbackUdata[i]);
//			}
//		}
//	}
}

std::string PresetHandler::recallPreset(int index)
{
	auto presetNameIt = mPresetsMap.find(index);
	if (presetNameIt != mPresetsMap.end()) {
		recallPreset(presetNameIt->second);
		return presetNameIt->second;
	} else {
		std::cout << "No preset index " << index << std::endl;
	}
	return "";
}

void PresetHandler::recallPresetSynchronous(std::string name)
{
	{
		if (mMorphRemainingSteps.load() >= 0) {
			mMorphRemainingSteps.store(-1);
			std::lock_guard<std::mutex> lk(mTargetLock);
		}
		mTargetValues = loadPresetValues(name);
	}
	for (ParameterMeta *param: mParameters) {
		if (mTargetValues.find(param->getFullAddress()) != mTargetValues.end()) { 
            setParameterValues(param, mTargetValues[param->getFullAddress()]);
		}
	}
	int index = -1;
	for (auto preset: mPresetsMap) {
		if (preset.second == name) {
			index = preset.first;
			break;
		}
	}
	mCurrentPresetName = name;
	if (mUseCallbacks) {
		for(size_t i = 0; i < mCallbacks.size(); ++i) {
			if (mCallbacks[i]) {
				mCallbacks[i](index, this, mCallbackUdata[i]);
			}
		}
	}
}

std::string PresetHandler::recallPresetSynchronous(int index)
{
	auto presetNameIt = mPresetsMap.find(index);
	if (presetNameIt != mPresetsMap.end()) {
		recallPresetSynchronous(presetNameIt->second);
		return presetNameIt->second;
	} else {
		std::cout << "No preset index " << index << std::endl;
	}
	return "";
}

std::map<int, std::string> PresetHandler::availablePresets()
{
	return mPresetsMap;
}

std::string PresetHandler::getPresetName(int index)
{
	return mPresetsMap[index];
}

void PresetHandler::skipParameter(std::string parameterAddr, bool skip) {
    std::unique_lock<std::mutex> lk(mSkipParametersLock);
    if (skip) {
        if (std::find(mSkipParameters.begin(), mSkipParameters.end(), parameterAddr) == mSkipParameters.end()) {
            mSkipParameters.push_back(parameterAddr);
        }
    } else {
        auto position = std::find(mSkipParameters.begin(), mSkipParameters.end(), parameterAddr);
        if (position != mSkipParameters.end()) {
            mSkipParameters.erase(position);
        }
    }
}

int PresetHandler::getCurrentPresetIndex() {
    std::map<int, std::string> presets = availablePresets();
    int current = -1;
    std::string currentPresetName = getCurrentPresetName();
    for (auto preset: presets) {
        if (preset.second == currentPresetName) {
            current = preset.first;
            break;
        }
    }
    return current;
}

float PresetHandler::getMorphTime()
{
    return mMorphTime.get();
}

void PresetHandler::setMorphTime(float time)
{
    mMorphTime.set(time);
}

void PresetHandler::stopMorph()
{
    {
        if (mMorphRemainingSteps.load() >= 0) {
            mMorphRemainingSteps.store(-1);
        }
    }
    {
        std::lock_guard<std::mutex> lk(mTargetLock);
        mMorphConditionVar.notify_all();
    }
}

std::string PresetHandler::getCurrentPath()
{
    std::string relPath = File::conformDirectory(getRootPath() + mSubDir);
	return relPath;
}

void PresetHandler::setRootPath(std::string path)
{
  assert(path.size() > 0);
  if (!File::exists(path)) {
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

std::string al::PresetHandler::getRootPath()
{
  std::string relPath = File::conformDirectory(mRootDir);
	return relPath;
}

void PresetHandler::print()
{
	std::cout << "Path: " << getCurrentPath() << std::endl;
	for (auto param: mParameters) {
		std::cout << param->getFullAddress() << std::endl;
	}
}

PresetHandler &PresetHandler::registerParameter(ParameterMeta &parameter)
{
    mParameters.push_back(&parameter);
    return *this;
}

PresetHandler &PresetHandler::registerParameterBundle(ParameterBundle &bundle)
{
    if (mBundles.find(bundle.name()) == mBundles.end()) {
        mBundles[bundle.name()] = std::vector<ParameterBundle *>();
    }
    mBundles[bundle.name()].push_back(&bundle);
    return *this;
}

std::map<int, std::string> PresetHandler::readPresetMap(std::string mapName)
{
	std::map<int, std::string> presetsMap;
	std::string mapFullPath = buildMapPath(mapName, true);
	std::ifstream f(mapFullPath);
	if (!f.is_open()) {
		if (mVerbose) {
			std::cout << "Error while opening preset map file for reading: " << mapFullPath << std::endl;
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
				std::cout << "End preset map."<< std::endl;
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
			std::cout << "Error while opening preset map file for reading: " << mFileName << std::endl;
		}
	}
	return presetsMap;
}

void PresetHandler::setCurrentPresetMap(std::string mapName, bool autoCreate)
{
	std::string mapFullPath = buildMapPath(mapName, true);
	if (autoCreate
	        && !File::exists(mapFullPath)
	        && !File::isDirectory(mapFullPath)) {
		std::cout << "No preset map. Creating default." << std::endl;
		std::vector<std::string> presets;
        // FIXME put back cehcking of directory contents
		// Dir presetDir(getCurrentPath());
		// while(presetDir.read()) {
		// 	FileInfo info = presetDir.entry();
		// 	if (info.type() == FileInfo::REG) {
		// 		std::string name = info.name();
		// 		if (name.substr(name.size()-7) == ".preset") {
		// 			presets.push_back(info.name().substr(0, name.size()-7));
		// 		}
		// 	}
		// }

		// get list of files ending in ".preset"
		FileList preset_files = filterInDir(getCurrentPath(), [](const FilePath& f){
			if (al::checkExtension(f, ".preset")) return true;
			else return false;
		});

		// store found preset files
		for (int i = 0; i < preset_files.count(); i += 1) {
			const FilePath& path = preset_files[i];
			const std::string& name = path.file();
			presets.push_back(name.substr(0, name.size()-7)); // exclude extension
		}

		for(size_t i = 0; i < presets.size(); ++i) {
			mPresetsMap[i] = presets[i];
		}
		mCurrentMapName = mapName;
		storeCurrentPresetMap();
    } else {
		mPresetsMap = readPresetMap(mapName);
		mCurrentMapName = mapName;
	}
    if (verbose()) {
      std::cout << "Setting preset map:" << mapName << std::endl;
    }
    for (auto cb:mPresetsMapCbs) {
        cb(mapName);
    }
}

void PresetHandler::changeParameterValue(std::string presetName,
                                         std::string parameterPath,
                                         float newValue)
{
        ParameterStates parameters = loadPresetValues(presetName);
	for (auto &parameter:parameters) {
		if (parameter.first == parameterPath) {
                        parameter.second[0] = newValue;
		}
	}
	savePresetValues(parameters, presetName, true);
}

void PresetHandler::storeCurrentPresetMap(std::string mapName, bool useSubDirectory)
{
    if (mapName.size() > 0) {
        mCurrentMapName = mapName;
    }
	std::string mapFullPath = buildMapPath(mCurrentMapName, useSubDirectory);
	std::ofstream f(mapFullPath);
	if (!f.is_open()) {
		if (mVerbose) {
			std::cout << "Error while opening preset map file: " << mapFullPath << std::endl;
		}
		return;
	}
	for(auto const &preset : mPresetsMap) {
		std::string line = std::to_string(preset.first) + ":" + preset.second;
		f << line << std::endl;
	}
	f << "::" << std::endl;
	if (f.bad()) {
		if (mVerbose) {
			std::cout << "Error while writing preset map file: " << mFileName << std::endl;
		}
	}
	f.close();
}

void PresetHandler::setParameterValues(ParameterMeta *p, std::vector<float> &values, double factor) {
    // We do a runtime check to determine the type of the parameter to determine how to draw it.
    if (strcmp(typeid(*p).name(), typeid(ParameterBool).name()) == 0) { // ParameterBool
        ParameterBool *param = dynamic_cast<ParameterBool *>(p);
        // No interpolation for parameter bool. Should we change exactly in the middle?
        param->set(values[0]);
    } else if (strcmp(typeid(*p).name(), typeid(Parameter).name()) == 0) {// Parameter
        Parameter *param = dynamic_cast<Parameter *>(p);
        float paramValue = param->get();
        float difference = values[0] - paramValue;
        //int steps = handler->mMorphRemainingSteps.load(); // factor = 1.0/steps
        if (factor > 0) {
            difference = difference * factor;
        }
        if (difference != 0.0) {
            float newVal = paramValue + difference;
            param->set(newVal);
        }
    } else if (strcmp(typeid(*p).name(), typeid(ParameterInt).name()) == 0) {// ParameterInt
        ParameterInt *param = dynamic_cast<ParameterInt *>(p);
//        int32_t paramValue = param->get();
//        double difference = values[0] - paramValue;
//        //int steps = handler->mMorphRemainingSteps.load(); // factor = 1.0/steps
//        if (factor > 0) {
//            difference = difference * factor;
//        }
//        if (difference != 0.0) {
//            int32_t newVal = std::round(paramValue + difference);
//            param->set(newVal);
//        }
        // The interpolation above is broken, no easy way to fix for things as they are now...
        param->set(int(values[0]));
    } else if (strcmp(typeid(*p).name(), typeid(ParameterPose).name()) == 0) {// Parameter pose
        ParameterPose *param = dynamic_cast<ParameterPose *>(p);
        if (values.size() == 7) {
            Pose paramValue = param->get();
            Pose difference;
           // TODO better interpolation of quaternion
            Vec3d differenceVec = Vec3d(values[0],values[1],values[2]) - paramValue.vec();
            Quatd differenceQuat = Quatd(values[3],values[4],values[5],values[6]) - paramValue.quat();
            //int steps = handler->mMorphRemainingSteps.load(); // factor = 1.0/steps
            if (factor > 0) {
                differenceVec = differenceVec * factor;
                differenceQuat = differenceQuat * factor;
            }
            if (differenceVec != Vec4f() && differenceQuat != Quatd()) {
                param->set(Pose(paramValue.vec() + differenceVec, paramValue.quat() + differenceQuat));
            }
        } else {
            std::cout << "Unexpected number of values for " << param->getFullAddress() << std::endl;
        }
    } else if (strcmp(typeid(*p).name(), typeid(ParameterMenu).name()) == 0) {// Parameter
        ParameterMenu *param = dynamic_cast<ParameterMenu *>(p);
        if (factor == 0) {
            param->set(values[0]);
        }
    } else if (strcmp(typeid(*p).name(), typeid(ParameterChoice).name()) == 0) {// Parameter
        ParameterChoice *param = dynamic_cast<ParameterChoice *>(p);
        if (factor == 0) {
            param->set((uint16_t) values[0]);
        }
    } else if (strcmp(typeid(*p).name(), typeid(ParameterVec3).name()) == 0) {// Parameter
        ParameterVec3 *param = dynamic_cast<ParameterVec3 *>(p);
        if (values.size() == 3) {
            Vec3f paramValue = param->get();
            Vec3f difference = Vec3f((float *)values.data()) - paramValue;
            //int steps = handler->mMorphRemainingSteps.load(); // factor = 1.0/steps
            if (factor > 0) {
                difference = difference * factor;
            }
            param->set(paramValue + difference);
        } else {
            std::cout << "Unexpected number of values for " << param->getFullAddress() << std::endl;
        }
    }  else if (strcmp(typeid(*p).name(), typeid(ParameterVec4).name()) == 0) {// Parameter
        ParameterVec4 *param = dynamic_cast<ParameterVec4 *>(p);
        if (values.size() == 4) {
            Vec4f paramValue = param->get();
            Vec4f difference = Vec4f((float *)values.data()) - paramValue;
            //int steps = handler->mMorphRemainingSteps.load(); // factor = 1.0/steps
            if (factor > 0) {
                difference = difference * factor;
            }
            param->set(paramValue + difference);
        } else {
            std::cout << "Unexpected number of values for " << param->getFullAddress() << std::endl;
        }
    } else if (strcmp(typeid(*p).name(), typeid(ParameterChoice).name()) == 0) {// Parameter
        ParameterChoice *param = dynamic_cast<ParameterChoice *>(p);
        if (factor == 0) {
            param->set(values[0]);
        }
    }  else if (strcmp(typeid(*p).name(), typeid(ParameterColor).name()) == 0) {// Parameter
        ParameterColor *param = dynamic_cast<ParameterColor *>(p);
        if (values.size() == 4) {
            Color paramValue = param->get();
            Color difference = Color(values[0],values[1],values[2],values[3]) - paramValue;
            //int steps = handler->mMorphRemainingSteps.load(); // factor = 1.0/steps
            if (factor > 0) {
                difference = difference * factor;
            }
            param->set(paramValue + difference);
        } else {
            std::cout << "Unexpected number of values for " << param->getFullAddress() << std::endl;
        }
    } else {
        std::cout << "Unsupported Parameter " << p->getFullAddress() << std::endl;
    }
}
void PresetHandler::setParametersInBundle(ParameterBundle *bundle, std::string bundlePrefix, PresetHandler *handler, float factor) {
    for (ParameterMeta *p : bundle->parameters()) {
//                       std::cout << bundlePrefix + p->getFullAddress() << std::endl;
        if (handler->mTargetValues.find(bundlePrefix + p->getFullAddress()) != handler->mTargetValues.end()) {
            handler->setParameterValues(p, handler->mTargetValues[bundlePrefix + p->getFullAddress()], factor);
        } else {
            if (handler->mVerbose) {
                std::cout << "Parameter not found " << bundlePrefix + p->getFullAddress() << std::endl;
            }
        }
    }
    for (auto subBundle: bundle->bundles()) {
        std::string subBundlePrefix = bundlePrefix + "/" + subBundle.second->name() + "/" + subBundle.first;
        handler->setParametersInBundle(subBundle.second, subBundlePrefix, handler, factor);
    }
}

void PresetHandler::morphingFunction(al::PresetHandler *handler)
{
	// handler->mMorphLock.lock();
	while(handler->mRunning) {
		std::unique_lock<std::mutex> lk(handler->mTargetLock);
		handler->mMorphConditionVar.wait(lk);
        int remainingSteps;
        while ((remainingSteps = std::atomic_fetch_sub(&(handler->mMorphRemainingSteps), 1)) > 0) {

            for (ParameterMeta *p : handler->mParameters) {
                if (handler->mTargetValues.find(p->getFullAddress()) != handler->mTargetValues.end()) {
                    handler->setParameterValues(p, handler->mTargetValues[p->getFullAddress()], 1.0/remainingSteps);
                } else {
                    if (handler->mVerbose) {
                        std::cout << "Parameter not found " << p->getFullAddress() << std::endl;
                    }
                }
            }
            for (auto bundleGroup : handler->mBundles) {
               const std::string &bundleName = bundleGroup.first;
               for (unsigned int i = 0; i < bundleGroup.second.size(); i++) {
                   std::string bundlePrefix = "/" + bundleName + "/" + std::to_string(i);
                   ParameterBundle *bundle = bundleGroup.second[i];
                   handler->setParametersInBundle(bundle, bundlePrefix, handler, 1.0/remainingSteps);
               }

            }
			al::wait(handler->mMorphInterval);
		}
//		// Set final values
//		for (Parameter param: mParameters) {
//			if (preset.find(param.getName()) != preset.end()) {
//				param.set(preset[param.getName()]);
//			}
//		}
	}
    // handler->mMorphLock.unlock();
}

PresetHandler::ParameterStates PresetHandler::getBundleStates(ParameterBundle *bundle, std::string id)
{
    ParameterStates values;
    std::string bundlePrefix = bundle->name() + "/" + id;
    for(ParameterMeta *p: bundle->parameters()) {
        values[bundlePrefix + p->getFullAddress()] = getParameterValue(p);
    }
    for(auto b: bundle->bundles()) {
        auto subBundleValues = getBundleStates(b.second, b.first);
        for (auto bundleValue: subBundleValues) {
            values[bundlePrefix + "/" + bundleValue.first ] = bundleValue.second;
        }
    }
    return values;
}

PresetHandler::ParameterStates PresetHandler::loadPresetValues(std::string name)
{
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
			std::cout << "Error while opening preset file: " << mFileName << std::endl;
		}
	}
	while(getline(f, line)) {
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
						std::cout << "End preset."<< std::endl;
					}
					break;
                }
				std::stringstream ss(line);
                std::string address, type;
                std::vector<float> values;
				std::getline(ss, address, ' ');
                std::getline(ss, type, ' ');
                std::string value;
                while (std::getline(ss, value, ' ')) {
                    values.push_back(std::stof(value));
                }

                if (address.size() > 0 && address[0] != '#' && type.size() > 0
                        && std::find(mSkipParameters.begin(), mSkipParameters.end(), address) == mSkipParameters.end()) {
                    // Address is not in ignore list, so add to values loaded
                    // Should we make sure the address corresponds to an existing preset?
                    preset[address] = values;
                }
			}
		}
	}
	if (f.bad()) {
		if (mVerbose) {
			std::cout << "Error while writing preset file: " << mFileName << std::endl;
		}
	}
	f.close();
	return preset;
}

bool PresetHandler::savePresetValues(const ParameterStates &values, std::string presetName,
                                      bool overwrite)
{
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
			std::cout << "Error while opening preset file: " << mFileName << std::endl;
		}
		return false;
	}
	f << "::" + presetName << std::endl;
	for(auto value: values) {
            std::string types, valueString;
            for (float &floatValue: value.second) {
                types += "f";
                valueString += std::to_string(floatValue) + " ";
            }
            // TODO chop last blank space
            std::string line = value.first + " " + types + " " + valueString;
            f << line << std::endl;
	}
	f << "::" << std::endl;
	if (f.bad()) {
		if (mVerbose) {
			std::cout << "Error while writing preset file: " << mFileName << std::endl;
		}
		ok = false;
	}
	f.close();
	return ok;
}

std::vector<float> PresetHandler::getParameterValue(ParameterMeta *p) {
    // We do a runtime check to determine the type of the parameter to determine how to draw it.
    if (strcmp(typeid(*p).name(), typeid(ParameterBool).name()) == 0) { // ParameterBool
        ParameterBool *param = dynamic_cast<ParameterBool *>(p);
        return std::vector<float>{ param->get() };
    } else if (strcmp(typeid(*p).name(), typeid(Parameter).name()) == 0) {// Parameter
        Parameter *param = dynamic_cast<Parameter *>(p);
        return std::vector<float>{ param->get() };
    } else if (strcmp(typeid(*p).name(), typeid(ParameterInt).name()) == 0) {// ParameterInt
        ParameterInt *param = dynamic_cast<ParameterInt *>(p);
        return std::vector<float>{ float(param->get()) };
    } else if (strcmp(typeid(*p).name(), typeid(ParameterPose).name()) == 0) {// Parameter pose
        ParameterPose *param = dynamic_cast<ParameterPose *>(p);
        Pose value = param->get();
        return std::vector<float>{ (float)value.pos()[0], (float)value.pos()[1], (float)value.pos()[2],
                    (float)value.quat().w, (float)value.quat().x, (float)value.quat().y, (float)value.quat().z };
    } else if (strcmp(typeid(*p).name(), typeid(ParameterMenu).name()) == 0) {// Parameter menu
        ParameterMenu *param = dynamic_cast<ParameterMenu *>(p);
        // TODO we should store the original int value, but float will do for now
        return std::vector<float>{ (float) param->get() };
    } else if (strcmp(typeid(*p).name(), typeid(ParameterChoice).name()) == 0) {// Parameter choice
        ParameterChoice *param = dynamic_cast<ParameterChoice *>(p);
        // TODO we should store the original int value, but float will do for now
        return std::vector<float>{ (float) param->get() };
    } else if (strcmp(typeid(*p).name(), typeid(ParameterVec3).name()) == 0) {// Parameter vec3
        ParameterVec3 *param = dynamic_cast<ParameterVec3 *>(p);
        Vec3f value = param->get();
        return std::vector<float>{ value.x, value.y,  value.z };
    }  else if (strcmp(typeid(*p).name(), typeid(ParameterVec4).name()) == 0) {// Parameter vec4
        ParameterVec4 *param = dynamic_cast<ParameterVec4 *>(p);
        Vec4f value = param->get();
        return std::vector<float>{ value.x, value.y,  value.z, value.w };
    } else if (strcmp(typeid(*p).name(), typeid(ParameterColor).name()) == 0) {// Parameter choice
        ParameterColor *param = dynamic_cast<ParameterColor *>(p);
        // TODO we should store the original int value, but float will do for now
        return std::vector<float>{ param->get().r, param->get().g, param->get().b, param->get().a };
    } else {
        // TODO this check should be performed on registration
        std::cout << "Unsupported Parameter type for storage for " << p->getFullAddress() << std::endl;
    }
    return std::vector<float>();
}


// PresetServer ----------------------------------------------------------------



PresetServer::PresetServer(std::string oscAddress, int oscPort) :
    mServer(nullptr), mOSCpath("/preset"),
    mAllowStore(true), mStoreMode(false),
    mNotifyPresetChange(true),
    mParameterServer(nullptr)
{
    mServer = new osc::Recv(oscPort, oscAddress.c_str(), 0.001); // Is this 1ms wait OK?
    if (mServer) {
        mServer->handler(*this);
        mServer->start();
    } else {
        std::cout << "Error starting OSC server." << std::endl;
    }
}


PresetServer::PresetServer(ParameterServer &paramServer) :
    mServer(nullptr), mOSCpath("/preset"),
    mAllowStore(true), mStoreMode(false),
    mNotifyPresetChange(true)
{
    paramServer.registerOSCListener(this);
    mParameterServer = &paramServer;
}

PresetServer::~PresetServer()
{
    //	std::cout << "~PresetServer()" << std::endl;;
    if (mServer) {
        mServer->stop();
        delete mServer;
        mServer = nullptr;
    }
}

void PresetServer::onMessage(osc::Message &m)
{
    m.resetStream(); // Should be moved to the caller...
    //	std::cout << "PresetServer::onMessage " << std::endl;
    mPresetChangeLock.lock();
    mPresetChangeSenderAddr = m.senderAddress();
    if(m.addressPattern() == mOSCpath && m.typeTags() == "f"){
        float val;
        m >> val;
        if (!this->mStoreMode) {
            for (PresetHandler *handler: mPresetHandlers) {
                handler->recallPreset(static_cast<int>(val));
            }
        } else {
            for (PresetHandler *handler: mPresetHandlers) {
                handler->storePreset(static_cast<int>(val));
            }
            this->mStoreMode = false;
        }
    } else if(m.addressPattern() == mOSCpath && m.typeTags() == "s"){
        std::string val;
        m >> val;
        if (!this->mStoreMode) {
            for (PresetHandler *handler: mPresetHandlers) {
                handler->recallPreset(val);
            }
        } else {
            for (PresetHandler *handler: mPresetHandlers) {
                handler->storePreset(val);
            }
            this->mStoreMode = false;
        }
    }  else if(m.addressPattern() == mOSCpath && m.typeTags() == "i"){
        int val;
        m >> val;
        if (!this->mStoreMode) {
            for (PresetHandler *handler: mPresetHandlers) {
                handler->recallPreset(val);
			}
		} else {
			for (PresetHandler *handler: mPresetHandlers) {
				handler->storePreset(val);
			}
			this->mStoreMode = false;
		}
	} else if (m.addressPattern() == mOSCpath + "/morphTime" && m.typeTags() == "f")  {
		float val;
		m >> val;
		for (PresetHandler *handler: mPresetHandlers) {
			handler->setMorphTime(val);
		}
	} else if (m.addressPattern() == mOSCpath + "/store" && m.typeTags() == "f")  {
		float val;
		m >> val;
		if (this->mAllowStore) {
			for (PresetHandler *handler: mPresetHandlers) {
				handler->storePreset(static_cast<int>(val));
			}
		}
	} else if (m.addressPattern() == mOSCpath + "/storeMode" && m.typeTags() == "f")  {
		float val;
		m >> val;
		if (this->mAllowStore) {
			this->mStoreMode = (val != 0.0f);
		} else {
			std::cout << "Remote storing disabled" << std::endl;
		}
	} else if (m.addressPattern() == mOSCpath + "/queryState")  {
		this->mParameterServer->notifyAll();
	} else if (m.addressPattern().substr(0, mOSCpath.size() + 1) == mOSCpath + "/") {
		int index = std::stoi(m.addressPattern().substr(mOSCpath.size() + 1));
		if (m.typeTags() == "f") {
			float val;
			m >> val;
			if (static_cast<int>(val) == 1) {
				if (!this->mStoreMode) {
					for (PresetHandler *handler: mPresetHandlers) {
						handler->recallPreset(index);
					}
				} else {
					for (PresetHandler *handler: mPresetHandlers) {
						handler->storePreset(index);
					}
					this->mStoreMode = false;
				}
			}
		}
	}
	mPresetChangeLock.unlock();
	mHandlerLock.lock();
	for(osc::PacketHandler *handler: mHandlers) {
		m.resetStream();
		handler->onMessage(m);
	}
	mHandlerLock.unlock();
}

void PresetServer::print()
{
	if (mServer) {
		std::cout << "Preset server listening on: " << mServer->address() << ":" << mServer->port() << std::endl;
		std::cout << "Communicating on path: " << mOSCpath << std::endl;

	} else {
		std::cout << "Preset Server Connected to shared server." << std::endl;
	}

    if (mOSCSenders.size() > 0) {
        std::cout << "Registered listeners: " << std::endl;
        for (auto sender:mOSCSenders) {
            std::cout << sender->address() << ":" << sender->port() << std::endl;
        }
    }
}

void PresetServer::stopServer()
{
	if (mServer) {
		mServer->stop();
		delete mServer;
		mServer = nullptr;
	}
}

bool PresetServer::serverRunning()
{
	if (mParameterServer) {
		return mParameterServer->serverRunning();
	} else {
		return (mServer != nullptr);
	}
}

void PresetServer::setAddress(std::string address)
{
	mOSCpath = address;
}

std::string PresetServer::getAddress()
{
	return mOSCpath;
}

void PresetServer::attachPacketHandler(osc::PacketHandler *handler)
{
	mHandlerLock.lock();
	mHandlers.push_back(handler);
	mHandlerLock.unlock();
}

void PresetServer::changeCallback(int value, void *sender, void *userData)
{
	(void) sender; // remove compiler warnings
	PresetServer *server = static_cast<PresetServer *>(userData);
//	Parameter *parameter = static_cast<Parameter *>(sender);

	if (server->mNotifyPresetChange) {
		server->notifyListeners(server->mOSCpath, value);
	}
}
