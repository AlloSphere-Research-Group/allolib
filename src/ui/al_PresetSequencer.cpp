
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
    : mRunning(false),
      mBeginCallbackEnabled(false),
      mEndCallbackEnabled(false),
      mSequencerThread(nullptr) {
  mTimeMasterMode = timeMasterMode;
  if (mTimeMasterMode == TimeMasterMode::TIME_MASTER_CPU) {
    startCpuThread();
  }
}

PresetSequencer::~PresetSequencer() { stopCpuThread(); }

void PresetSequencer::playSequence(std::string sequenceName, double timeScale) {
  stopSequence();
  mSequenceLock.lock();

  if (sequenceName.size() > 0) {
    auto steps = loadSequence(sequenceName, timeScale);
    mCurrentSequence = sequenceName;
    mSteps = steps;
  }

  // Initialize counters
  //  mSequenceStart = std::chrono::high_resolution_clock::now();
  mCurrentTime = 0.0;
  mCurrentStep = 0;
  mLastTimeUpdate = 0.0;
  mTargetTime = 0.0;
  mRunning = false;
  mSequenceLock.unlock();

  if (mTimeMasterMode == TimeMasterMode::TIME_MASTER_CPU) {
    {
      //      std::unique_lock<std::mutex> lk2(mPlayStartedLock);
      mPlayPromiseObj = std::make_shared<std::promise<void>>();
      auto playFuture = mPlayPromiseObj->get_future();
      {
        std::unique_lock<std::mutex> lk(mPlayWaitLock);
        mPlayWaitVariable.notify_all();
      }
      playFuture.get();
      //      mPlayStartedVariable.wait(lk2);
    }

  } else {
    mRunning = true;
  }
}

void PresetSequencer::stopSequence(bool triggerCallbacks) {
  if (mRunning == true) {
    bool mCallbackStatus = false;
    if (!triggerCallbacks) {
      mCallbackStatus = mEndCallbackEnabled;
      enableEndCallback(false);
    }
    if (!triggerCallbacks) {
      enableEndCallback(mCallbackStatus);
    }
    mRunning = false;
  }
  //  if (mSequencerThread) {
  //    //    std::unique_lock<std::mutex> lk2(mPlayStartedLock);
  //    mPlayPromiseObj = std::make_shared<std::promise<bool>>();
  //    auto playFuture = mPlayPromiseObj->get_future();
  //    std::unique_lock<std::mutex> lk(mPlayWaitLock);
  //    mPlayWaitVariable.notify_all();
  //    //    playFuture.wait();
  //    //    mPlayStartedVariable.wait(lk2);
  //  }
}

void PresetSequencer::setTime(double time) {
  if (running()) {
    mTimeRequest = time;
  } else {
    //    mSteps = mMostRecentSequence;
    double currentTime = 0.0;
    auto sequencer = this;
    auto timeRequest = time;
    mCurrentStep = 0;
    if (mSteps.size() > 0) {
      Step step = sequencer->mSteps[mCurrentStep];
      std::string previousPreset = mSteps.front().presetName;
      while (currentTime < timeRequest &&
             mCurrentStep < sequencer->mSteps.size()) {
        step = sequencer->mSteps[mCurrentStep];
        currentTime += step.morphTime + step.waitTime;
        //                std::cout << "Skipping: " << step.presetName << " " <<
        //                step.morphTime << ":" << step.waitTime << std::endl;
        if (currentTime < timeRequest) {
          previousPreset = sequencer->mSteps.front().presetName;
        }
        mCurrentStep++;
      }
      if (timeRequest >
          (currentTime -
           step.waitTime)) {  // We only need to wait, morphing is done
        //                        sequencer->mPresetHandler->setMorphTime(0);
        sequencer->mPresetHandler->recallPresetSynchronous(step.presetName);
        sequencer->mPresetHandler->setMorphTime(
            step.morphTime);  // Just set it so it has the expected last value
        //                targetTime = now + std::chrono::microseconds(int(1.0e6
        //                * (currentTime - timeRequest))); sequenceStart = now -
        //                std::chrono::microseconds(int(1.0e6 * (timeRequest)));
      } else {  // We need to finish the morphing
        float remainingMorphTime = currentTime - timeRequest - step.waitTime;
        if (previousPreset.size() > 0) {
          //                            sequencer->mPresetHandler->recallPresetSynchronous(previousPreset);

          sequencer->mPresetHandler->setInterpolatedPreset(
              previousPreset, step.presetName,
              1.0 - (remainingMorphTime / step.morphTime));
          //                    std::cout << "Interpolating: " << previousPreset
          //                    << " " << step.presetName << " " << 1.0 -
          //                    (remainingMorphTime/step.morphTime) <<
          //                    std::endl;
        }
        //                sequencer->mPresetHandler->setMorphTime(remainingMorphTime);
        //                sequencer->mPresetHandler->recallPreset(step.presetName);
        //                targetTime = now + std::chrono::microseconds(int(1.0e6
        //                * (currentTime - timeRequest))); sequenceStart = now -
        //                std::chrono::microseconds(int(1.0e6 * (timeRequest)));
      }
      if (sequencer->mTimeChangeCallback) {
        //                    std::cout <<  1.0e-9 *
        //                    std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()
        //                    - sequenceStart).count() <<std::endl;
        sequencer->mTimeChangeCallback(time);
      }
    }
  }
}

void PresetSequencer::rewind() {
  if (mSequenceLock.try_lock()) {
    mCurrentStep = 0;
    setTime(0.0);
    mSequenceLock.unlock();
  } else {  // If lock can't be acquired, request time change
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
    {
      std::unique_lock<std::mutex> lk(sequencer->mPlayWaitLock);
      sequencer->mPlayWaitVariable.wait(lk);
      if (sequencer->mBeginCallbackEnabled &&
          sequencer->mBeginCallback != nullptr) {
        sequencer->mBeginCallback(sequencer, sequencer->mBeginCallbackData);
      }
      std::cout << "After begin callback" << std::endl;
      sequencer->mRunning = true;
      sequencer->mStartRunning = false;
      sequencer->mPlayPromiseObj->set_value();
    }
    //    {
    //      std::unique_lock<std::mutex> lk(sequencer->mPlayStartedLock);
    //      sequencer->mPlayStartedVariable.notify_all();
    //    }
    // to have the calling function know play has started
    // and callbacks have been called.
    //    sequencer->mSequenceLock.lock();

    std::cout << "running" << std::endl;

    while (sequencer->running()) {
      sequencer->stepSequencer(sequencer->mGranularity / 1000.0);
      std::this_thread::sleep_for(
          std::chrono::milliseconds(sequencer->mGranularity));
    }
    sequencer->mRunning = false;
    std::cout << "Sequence finished." << std::endl;
    if (sequencer->mPresetHandler) {
      std::this_thread::sleep_for(std::chrono::milliseconds(
          100));  // Give a little time to process any pending preset changes.
      sequencer->mPresetHandler->stopMorph();
    }
    //    sequencer->mSequenceLock.unlock();
    if (sequencer->mEndCallbackEnabled && sequencer->mEndCallback != nullptr) {
      bool finished = sequencer->mSteps.size() == 0;
      sequencer->mEndCallback(finished, sequencer, sequencer->mEndCallbackData);
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

PresetSequencer &PresetSequencer::registerPresetHandler(
    PresetHandler &presetHandler) {
  mPresetHandler = &presetHandler;
  // We need to take over the preset handler timing.
  mPresetHandler->setTimeMaster(TimeMasterMode::TIME_MASTER_CPU);
  mDirectory = mPresetHandler->getCurrentPath();
  //		std::cout << "Path set to:" << mDirectory << std::endl;
  return *this;
}

std::vector<PresetSequencer::Step> PresetSequencer::loadSequence(
    std::string sequenceName, double timeScale) {
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
      step.presetName = name.substr(1);  // chop initial '@'
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
      step.presetName = delta;
      step.waitTime = std::stof(name.substr(1)) * timeScale;
      step.params = {float(std::stod(duration) * timeScale)};
      steps.push_back(step);
      // std::cout << name  << ":" << delta << ":" << duration << std::endl;
    } else if (name.size() > 0 && name[0] != '#' && name[0] != '\r') {
      Step step;
      step.type = PRESET;
      step.presetName = name;
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
    std::function<void(void *, std::vector<float> &params)> callback,
    void *data) {
  EventCallback cb;
  cb.eventName = eventName;
  cb.callback = callback;
  cb.callbackData = data;

  mEventCallbacks.push_back(cb);
}

void PresetSequencer::registerBeginCallback(
    std::function<void(PresetSequencer *sender, void *userData)> beginCallback,
    void *userData) {
  mBeginCallback = beginCallback;
  mBeginCallbackData = userData;
  mBeginCallbackEnabled = true;
}

void PresetSequencer::registerEndCallback(
    std::function<void(bool, al::PresetSequencer *, void *)> endCallback,
    void *userData) {
  // FIXME this data needs to be protected with a mutex
  mEndCallback = endCallback;
  mEndCallbackData = userData;
  mEndCallbackEnabled = true;
}

void PresetSequencer::registerTimeChangeCallback(
    std::function<void(float)> func, float minTimeDeltaSec) {
  if (!mRunning && mSequenceLock.try_lock()) {
    mTimeChangeMinTimeDelta = minTimeDeltaSec;
    mTimeChangeCallback = func;
    mSequenceLock.unlock();
  } else {
    std::cerr << "ERROR: Failed to set time change callback. Sequencer running"
              << std::endl;
  }
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
  for (auto &step : steps) {
    duration += step.morphTime + step.waitTime;
  }
  return duration;
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
  if (timeRequest > 0.0) {
    std::cout << "Requested " << timeRequest << std::endl;
    mCurrentTime = 0.0;
    mCurrentStep = 0;
    if (mCurrentTime < timeRequest) {
      std::string previousPreset = mSteps.front().presetName;
      while (mCurrentTime < timeRequest && mSteps.size() > mCurrentStep) {
        Step step = mSteps[mCurrentStep];
        if (step.type == PRESET) {
          mCurrentTime += step.morphTime + step.waitTime;
          //            std::cout << "Skipping: " << step.presetName << " "
          //                      << step.morphTime << ":" << step.waitTime <<
          //                      std::endl;
          if (mCurrentTime < timeRequest) {
            previousPreset = mSteps.front().presetName;
          }
        }
        mCurrentStep++;
        //        mSteps.pop();
      }
      Step step = mSteps.front();
      if (timeRequest > (mCurrentTime - step.waitTime)) {
        // We only need to wait a bit after morphing is done
        mPresetHandler->recallPresetSynchronous(step.presetName);
        mPresetHandler->setMorphTime(
            step.morphTime);  // No effect, but just set it so it has the
                              // expected last value
        mCurrentTime = timeRequest;
        mTargetTime = mCurrentTime - timeRequest;
        //          mSequenceStart =
        //              std::chrono::high_resolution_clock::now() -
        //              std::chrono::microseconds(int(1.0e6 * (timeRequest)));
      } else {  // We need to finish the morphing
        float remainingMorphTime = mCurrentTime - timeRequest - step.waitTime;
        if (previousPreset.size() > 0) {
          //                            sequencer->mPresetHandler->recallPresetSynchronous(previousPreset);

          mPresetHandler->setInterpolatedPreset(
              previousPreset, step.presetName,
              1.0 - (remainingMorphTime / step.morphTime));
          std::cout << "Interpolating: " << previousPreset << " "
                    << step.presetName << " "
                    << 1.0 - (remainingMorphTime / step.morphTime) << std::endl;
        }
        mPresetHandler->setMorphTime(remainingMorphTime);
        mPresetHandler->recallPreset(step.presetName);
        mTargetTime = mCurrentTime - timeRequest;
        mCurrentTime = timeRequest;
        //          mSequenceStart =
        //              std::chrono::high_resolution_clock::now() -
        //              std::chrono::microseconds(int(1.0e6 * (timeRequest)));
      }
    }

    //    mLastTimeUpdate = timeRequest;
  }
}

void PresetSequencer::stepSequencer(double dt) {
  mCurrentTime += dt;
  if (mRunning) {
    mSequenceLock.lock();
    // Process parameter change

    //    {
    //      // Need to process next events
    //      Step step = mSteps.front();
    //      std::cout << "next event " << step.presetName << std::endl;
    //      if (step.type == EVENT) {  // After event is triggered, call
    //      callback
    //        for (auto eventCallback : mEventCallbacks) {
    //          if (eventCallback.eventName == step.presetName) {
    //            eventCallback.callback(eventCallback.callbackData,
    //            step.params); break;
    //          }
    //        }
    //      }
    //    }

    processTimeChangeRequest();

    // Process time callback
    if (mTimeChangeCallback) {
      if ((mCurrentTime - mLastTimeUpdate) >= mTimeChangeMinTimeDelta) {
        mLastTimeUpdate = mCurrentTime + mTimeChangeMinTimeDelta;
        mTimeChangeCallback(float(mCurrentTime));
      }
    }

    while (mTargetTime < mCurrentTime && (mSteps.size() > mCurrentStep)) {
      // Reached target time. Process step
      if (mSteps.size() > mCurrentStep) {
        Step step = mSteps[mCurrentStep];
        assert(step.type == PRESET);
        assert(mParameterList.empty());
        //        while (!mParameterList.empty()) {
        //            mParameterList.pop();
        //        }
        if (step.type == PRESET) {
          if (mPresetHandler) {
            mPresetHandler->setMorphTime(step.morphTime);
            mPresetHandler->recallPreset(step.presetName);
            std::cout << "recalling " << step.presetName << std::endl;
          } else {
            std::cerr << "No preset handler registered with PresetSequencer. "
                         "Ignoring preset change."
                      << std::endl;
          }
          mCurrentStep++;
          mTargetTime += step.morphTime + step.waitTime;
        } else if (step.type == PARAMETER) {
          mCurrentStep++;
          mTargetTime += step.morphTime + step.waitTime;
        } else if (step.type == EVENT) {
        }

        //        if (!mRunning) {
        //            // Not running but events still in sequence. Means stop
        //            has been
        //            // requested.
        //            if (mEndCallback) {
        //                mEndCallback(false, this, mEndCallbackData);
        //            }
        //            mTargetTime = mCurrentTime;
        //            while (!mParameterList.empty()) {
        //                mParameterList.pop();
        //            }
        //        }
      }
    }
    if (mTargetTime <= mCurrentTime) {
      mRunning = false;
    }
    mSequenceLock.unlock();
  }
}

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
  std::string fullName = mDirectory;
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
    mPresetHandler->stopMorph();
  }

  if (mSequencerThread) {
    mSequencerActive = false;
    mPlayPromiseObj = std::make_shared<std::promise<void>>();
    auto playFuture = mPlayPromiseObj->get_future();
    {
      std::unique_lock<std::mutex> lk(mPlayWaitLock);
      mPlayWaitVariable.notify_all();
    }
    playFuture.get();
    mRunning = false;
    mSequencerThread->join();
    mSequencerThread = nullptr;
  }
}
