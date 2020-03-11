
#include "al/ui/al_PresetSequencer.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "al/io/al_File.hpp"
#include "al/ui/al_Composition.hpp"
#include "al/ui/al_SequenceRecorder.hpp"

using namespace al;

PresetSequencer::PresetSequencer(TimeMasterMode timeMasterMode)
    : mRunning(false), mBeginCallbackEnabled(false), mEndCallbackEnabled(false),
      mSequencerThread(nullptr) {
  mTimeMasterMode = timeMasterMode;
  if (mTimeMasterMode == TimeMasterMode::TIME_MASTER_CPU) {
    startCpuThread();
  }
  if (mTimeMasterMode == TimeMasterMode::TIME_MASTER_GRAPHICS ||
      mTimeMasterMode == TimeMasterMode::TIME_MASTER_AUDIO) {
    std::cerr << "ERROR: PresetSequencer: TimeMasterMode not supported, "
                 "treating as TIME_MASTER_CPU"
              << std::endl;
  }
}

PresetSequencer::~PresetSequencer() { stopCpuThread(); }

void PresetSequencer::playSequence(std::string sequenceName, double timeScale,
                                   double timeOffset) {
  stopSequence();
  mSequenceLock.lock();

  if (sequenceName.size() > 0) {
    auto steps = loadSequence(sequenceName, timeScale);
    mCurrentSequence = sequenceName;
    mSteps = steps;
  }
  mSequenceLock.unlock();

  // Initialize counters
  mLastTimeUpdate = 0.0;
  mParameterTargetTime = 0.0;
  mStartRunning = true;
  mRunning = false;
  setTime(0.0);
  if (mSteps.size() > 0 && mSteps.size() > mCurrentStep) {
    if (mPresetHandler && mSteps[mCurrentStep].type == PRESET) {
      mPresetHandler->morphTo(mSteps[mCurrentStep].name,
                              mSteps[mCurrentStep].morphTime);
    }
  }
  setTime(timeOffset);

  if (mTimeMasterMode == TimeMasterMode::TIME_MASTER_CPU && mSequencerThread) {
    {
      mPlayPromiseObj = std::make_shared<std::promise<void>>();
      auto playFuture = mPlayPromiseObj->get_future();
      {
        std::unique_lock<std::mutex> lk(mPlayWaitLock);
        mPlayWaitVariable.notify_all();
      }
      playFuture.get();
    }

  } else {
    mStartRunning = false;
    mRunning = true;
  }
}

void PresetSequencer::stopSequence(bool triggerCallbacks) {
  if (mRunning == true) {
    bool previousCallbackStatus = false;
    if (!triggerCallbacks) {
      previousCallbackStatus = mEndCallbackEnabled;
      enableEndCallback(false);
    }
    mRunning = false;
    if (mTimeMasterMode == TimeMasterMode::TIME_MASTER_CPU) {
      std::unique_lock<std::mutex> lk(mPlayWaitLock);
    }

    if (mPresetHandler) {
      mPresetHandler->stopMorphing();
    }
    // Wait until CPU thread is waiting to start again.
    if (!triggerCallbacks) {
      enableEndCallback(previousCallbackStatus);
    }
  }
}

void PresetSequencer::setTime(double time) {
  if (running()) {
    mTimeRequest = time;
  } else {
    updateTime(time);
  }
}

void PresetSequencer::rewind() {
  if (mSequenceLock.try_lock()) {
    setTime(0.0);
    mSequenceLock.unlock();
  } else { // If lock can't be acquired, request time change
    setTime(0.0);
  }
}

std::vector<std::string> al::PresetSequencer::getSequenceList() {
  std::vector<std::string> sequenceList;
  std::string path = mDirectory;
  if (mPresetHandler) {
    path = mPresetHandler->getCurrentPath();
  }
  if (!File::isDirectory(path)) {
    Dir::make(path);
  }

  // get list of files ending in ".sequence"
  FileList sequence_files = filterInDir(path, [](const FilePath &f) {
    if (al::checkExtension(f, ".sequence"))
      return true;
    else
      return false;
  });

  // store found preset files
  for (int i = 0; i < sequence_files.count(); i += 1) {
    const FilePath &path = sequence_files[i];
    const std::string &name = path.file();
    // exclude extension when adding to sequence list
    sequenceList.push_back(name.substr(0, name.size() - 9));
  }
  std::sort(sequenceList.begin(), sequenceList.end());

  return sequenceList;
}

void PresetSequencer::sequencerFunction(al::PresetSequencer *sequencer) {
  while (sequencer->mSequencerActive) {
    //    {
    std::unique_lock<std::mutex> lk(sequencer->mPlayWaitLock);
    sequencer->mPlayWaitVariable.wait(lk);
    if (!sequencer->mStartRunning) {
      sequencer->mPlayPromiseObj->set_value();
      return;
    }
    if (sequencer->mBeginCallbackEnabled &&
        sequencer->mBeginCallback != nullptr) {
      sequencer->mBeginCallback(sequencer);
    }
    //      std::cout << "After begin callback" << std::endl;
    sequencer->mRunning = true;
    sequencer->mStartRunning = false;
    sequencer->mPlayPromiseObj->set_value();
    //    }

    while (sequencer->running()) {
      sequencer->stepSequencer(sequencer->mGranularity * 1.0e-9);
      std::this_thread::sleep_for(
          std::chrono::nanoseconds(sequencer->mGranularity));
    }
    //    std::cout << "Sequence finished." << std::endl;
    if (sequencer->mPresetHandler) {
      std::this_thread::sleep_for(std::chrono::milliseconds(
          100)); // Give a little time to process any pending preset changes.
      sequencer->mPresetHandler->stopMorphing();
    }
    if (sequencer->mEndCallbackEnabled && sequencer->mEndCallback != nullptr) {
      bool finished = sequencer->mSteps.size() == sequencer->mCurrentStep;
      sequencer->mEndCallback(finished, sequencer);
    }
  }
}

void PresetSequencer::setHandlerSubDirectory(std::string subDir) {
  if (mPresetHandler) {
    mPresetHandler->setSubDirectory(subDir);
  } else {
    std::cerr << "Error in PresetSequencer::setHandlerSubDirectory. "
                 "PresetHandler not registered."
              << std::endl;
  }
}

PresetSequencer &
PresetSequencer::registerPresetHandler(PresetHandler &presetHandler) {
  mPresetHandler = &presetHandler;
  // We need to take over the preset handler timing.
  //  mPresetHandler->setTimeMaster(TimeMasterMode::TIME_MASTER_CPU);
  mDirectory = mPresetHandler->getCurrentPath();
  //		std::cout << "Path set to:" << mDirectory << std::endl;
  return *this;
}

std::vector<PresetSequencer::Step>
PresetSequencer::loadSequence(std::string sequenceName, double timeScale) {
  std::vector<Step> steps;
  std::string fullName = buildFullPath(sequenceName);
  std::ifstream f(fullName);
  if (!f.is_open()) {
    std::cout << "Could not open:" << fullName << std::endl;
    return steps;
  }

  std::string line;
  while (getline(f, line)) {
    if (line.substr(0, 2) == "::") {
      break;
    }
    // FIXME here and in other sequencers white space should be stripped out
    std::stringstream ss(line);
    std::string name, delta, duration;
    std::getline(ss, name, ':');
    std::getline(ss, delta, ':');
    std::getline(ss, duration, ':');
    if (name.size() > 0 && name[0] == '@') {
      Step step;
      step.type = EVENT;
      step.name = name.substr(1); // chop initial '@'
      step.morphTime = std::stof(delta) * timeScale;
      step.waitTime = std::stof(duration) * timeScale;

      // FIXME allow any number or parameters
      std::string next;
      std::getline(ss, next, ':');
      step.params.push_back(std::stof(next));
      steps.push_back(step);
      //			 std::cout << name  << ":" << delta << ":" <<
      // duration << std::endl;
    } else if (name.size() > 0 && name[0] == '+') {
      Step step;
      step.type = PARAMETER;
      step.name = delta;
      step.waitTime = std::stof(name.substr(1)) * timeScale;
      step.params = {float(std::stod(duration) * timeScale)};
      steps.push_back(step);
      // std::cout << name  << ":" << delta << ":" << duration << std::endl;
    } else if (name.size() > 0 && name[0] != '#' && name[0] != '\r') {
      Step step;
      step.type = PRESET;
      step.name = name;
      step.morphTime = std::stof(delta) * timeScale;
      step.waitTime = std::stof(duration) * timeScale;
      steps.push_back(step);
      // std::cout << name  << ":" << delta << ":" << duration << std::endl;
    }
  }
  if (f.bad()) {
    std::cout << "Error reading:" << sequenceName << std::endl;
  }
  return steps;
}

void PresetSequencer::registerEventCommand(
    std::string eventName,
    std::function<void(void *, std::vector<ParameterField> &)> callback,
    void *data) {
  EventCallback cb;
  cb.eventName = eventName;
  cb.callback = callback;
  cb.callbackData = data;

  mEventCallbacks.push_back(cb);
}

void PresetSequencer::registerBeginCallback(
    std::function<void(PresetSequencer *sender)> beginCallback) {
  mBeginCallback = beginCallback;
  mBeginCallbackEnabled = true;
}

void PresetSequencer::registerEndCallback(
    std::function<void(bool, al::PresetSequencer *)> endCallback) {
  // FIXME this data needs to be protected with a mutex
  mEndCallback = endCallback;
  mEndCallbackEnabled = true;
}

void PresetSequencer::registerTimeChangeCallback(
    std::function<void(float)> func, float minTimeDeltaSec) {
  if (!mRunning && mSequenceLock.try_lock()) {
    mTimeChangeMinTimeDelta = minTimeDeltaSec;
    mTimeChangeCallbacks.push_back(func);
    mSequenceLock.unlock();
  } else {
    std::cerr << "ERROR: Failed to set time change callback. Sequencer running"
              << std::endl;
  }
}

float PresetSequencer::getSequenceStartOffset(std::string sequenceName) {
  std::vector<Step> steps;
  if (sequenceName == mCurrentSequence) {
    mSequenceLock.lock();
    steps = mSteps;
    mSequenceLock.unlock();
  } else {
    steps = loadSequence(sequenceName);
  }
  float additionalDuration = 0.0f;
  for (auto &step : steps) {
    if (step.type == PRESET) {
      break;
    } else {
      additionalDuration += step.waitTime;
    }
  }
  return additionalDuration;
}

float PresetSequencer::getSequenceTotalDuration(std::string sequenceName) {
  std::vector<Step> steps;
  if (sequenceName == mCurrentSequence) {
    mSequenceLock.lock();
    steps = mSteps;
    mSequenceLock.unlock();
  } else {
    steps = loadSequence(sequenceName);
  }
  float duration = 0.0f;
  float additionalDuration = 0.0f;
  for (auto &step : steps) {
    if (step.type == PRESET) {
      duration += step.morphTime + step.waitTime;
      additionalDuration = 0.0;
    } else {
      additionalDuration += step.waitTime;
    }
  }
  return duration + additionalDuration;
}

void PresetSequencer::clearSteps() {
  stopSequence();
  mSequenceLock.lock();
  mSteps.clear();
  mCurrentSequence = "";
  mSequenceLock.unlock();
}

void PresetSequencer::appendStep(PresetSequencer::Step &newStep) {
  mSequenceLock.lock();
  mSteps.push_back(newStep);
  mCurrentSequence = "";
  mSequenceLock.unlock();
}

void PresetSequencer::setTimeMaster(TimeMasterMode masterMode) {
  if (masterMode == TimeMasterMode::TIME_MASTER_CPU) {
    startCpuThread();
  } else {
    if (mTimeMasterMode == TimeMasterMode::TIME_MASTER_CPU) {
      stopCpuThread();
    }
  }
  mTimeMasterMode = masterMode;
}

void PresetSequencer::processTimeChangeRequest() {
  // Process time change request
  double timeRequest = double(mTimeRequest.exchange(-1.0f));
  if (timeRequest >= 0.0) {
    updateTime(timeRequest);
  }
}

void PresetSequencer::updateTime(double time) {
  //    mSteps = mMostRecentSequence;
  if (mSteps.size() > 0) {
    mCurrentStep = 0;
    mCurrentTime = time;
    mTargetTime = 0;
    mLastPresetTime = 0.0;
    mLastTimeUpdate = 0.0;
    while (!mParameterList.empty()) {
      mParameterList.pop();
    }

    // Queue parameter and event steps before first preset.
    while ((mSteps.size() > mCurrentStep) &&
           (mSteps[mCurrentStep].type != PRESET)) {
      mParameterList.push(mSteps[mCurrentStep]);
      // Move current time back to accomodate these steps
      mCurrentTime -= mSteps[mCurrentStep].waitTime;
      mLastPresetTime = mCurrentTime;
      mParameterTargetTime = mLastPresetTime;
      mLastTimeUpdate = mParameterTargetTime;
      mCurrentStep++;
    }

    updateSequencer();
    if (mSteps.size() > (mCurrentStep - 1) && mCurrentStep > 1) {
      Step step = mSteps[mCurrentStep - 1];
      std::string previousPreset = mSteps[mCurrentStep - 2].name;
      if (time > (mTargetTime - step.waitTime)) {
        // We only need to wait, morphing is done
        if (mPresetHandler) {
          mPresetHandler->recallPresetSynchronous(step.name);
          // Just set morph time so it has the expected last value
          mPresetHandler->setMorphTime(step.morphTime);
        }
      } else {
        // In the middle of morphing
        if (mPresetHandler) {
          double remainingMorphTime =
              mTargetTime - time - double(step.waitTime);
          if (previousPreset.size() > 0) {
            mPresetHandler->setInterpolatedPreset(
                previousPreset, step.name,
                1.0 - (remainingMorphTime / step.morphTime));
          }
          if (mRunning || mStartRunning) {
            mPresetHandler->morphTo(step.name, remainingMorphTime);
          }
        }
      }
    }

    for (auto cb : mTimeChangeCallbacks) {
      cb(time);
    }
  }
}

void PresetSequencer::updateSequencer() {
  mSequenceLock.lock();

  while (mTargetTime <= mCurrentTime && (mSteps.size() > mCurrentStep)) {
    // Reached target time. Process step
    Step step = mSteps[mCurrentStep];
    assert(step.type == PRESET);

    if (mPresetHandler && (mRunning || mStartRunning)) {
      mPresetHandler->morphTo(step.name, step.morphTime);
      //            std::cout << "recalling " << step.presetName <<
      //            std::endl;
    }
    mLastPresetTime = mTargetTime;
    mParameterTargetTime = mTargetTime;
    mTargetTime += step.morphTime + step.waitTime;
    mCurrentStep++;
    // Now gather all parameter and event steps until next preset
    while ((mSteps.size() > mCurrentStep) &&
           mSteps[mCurrentStep].type != PRESET) {
      mParameterList.push(mSteps[mCurrentStep]);
      mCurrentStep++;
    }
  }
  // Apply pending parameter changes
  while (!mParameterList.empty()) {
    auto &step = mParameterList.front();
    if (mParameterTargetTime <= mCurrentTime) {
      if (step.type == PARAMETER) {
        //          std::cout << "set parameter " << step.presetName <<
        //          std::endl;
        for (auto *param : mParameters) {
          if (param->getFullAddress() == step.name) {
            param->setFields(step.params);
            break;
          }
        }
      } else if (step.type == EVENT) {
        for (auto &eventCallback : mEventCallbacks) {
          if (eventCallback.eventName == step.name) {
            eventCallback.callback(eventCallback.callbackData, step.params);
          }
        }
      }
      mParameterList.pop();
      if (!mParameterList.empty()) {
        mParameterTargetTime += mParameterList.front().waitTime;
      }
    } else {
      break;
    }
  }
  if (mTargetTime <= mCurrentTime && mParameterList.empty() &&
      mParameterTargetTime <= mCurrentTime) {
    mRunning = false;
  }
  mSequenceLock.unlock();
}

void PresetSequencer::stepSequencer(double dt) {
  mCurrentTime += dt;
  if (mRunning) {
    processTimeChangeRequest();

    // Process time callback
    if ((mCurrentTime - mLastTimeUpdate) >= mTimeChangeMinTimeDelta) {
      mLastTimeUpdate = mCurrentTime + mTimeChangeMinTimeDelta;
      for (auto cb : mTimeChangeCallbacks) {
        cb(float(mCurrentTime));
      }
    }
    updateSequencer();
  }
}

void PresetSequencer::stepSequencer() { stepSequencer(mGranularity * 1.0e-9); }

bool PresetSequencer::consumeMessage(osc::Message &m, std::string rootOSCPath) {
  std::string basePath = rootOSCPath;
  if (mOSCsubPath.size() > 0) {
    basePath += "/" + mOSCsubPath;
  }
  if (m.addressPattern() == basePath && m.typeTags() == "s") {
    std::string val;
    m >> val;
    std::cout << "start sequence " << val << std::endl;
    playSequence(val);
    return true;
  } else if (m.addressPattern() == basePath + "/stop") {
    std::cout << "stop sequence " << std::endl;
    stopSequence();
    return true;
  }
  return false;
}

std::string PresetSequencer::buildFullPath(std::string sequenceName) {
  std::string fullName = File::conformDirectory(mDirectory);
  if (mPresetHandler) {
    fullName = File::conformDirectory(mPresetHandler->getCurrentPath());
  }
  if (sequenceName.size() < 9 ||
      sequenceName.substr(sequenceName.size() - 9) != ".sequence") {
    sequenceName += ".sequence";
  }
  fullName += sequenceName;
  return fullName;
}

void PresetSequencer::startCpuThread() {
  stopCpuThread();
  if (mTimeMasterMode == TimeMasterMode::TIME_MASTER_CPU) {
    mSequencerActive = true;
    mSequencerThread =
        std::make_unique<std::thread>(PresetSequencer::sequencerFunction, this);
  }
}

void PresetSequencer::stopCpuThread() {
  stopSequence(false);
  if (mPresetHandler) {
    mPresetHandler->stopCpuThread();
  }

  if (mSequencerThread) {
    mSequencerActive = false;
    mRunning = false;
    mPlayPromiseObj = std::make_shared<std::promise<void>>();
    auto playFuture = mPlayPromiseObj->get_future();
    {
      std::unique_lock<std::mutex> lk(mPlayWaitLock);
      mPlayWaitVariable.notify_all();
    }
    playFuture.get();
    mSequencerThread->join();
    mSequencerThread = nullptr;
  }
}
