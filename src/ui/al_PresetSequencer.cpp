
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
      mStartingRun(false),
      mSequencerThread(nullptr),
      mBeginCallbackEnabled(false),
      mEndCallbackEnabled(false) {
  mTimeMasterMode = timeMasterMode;
  if (mTimeMasterMode == TimeMasterMode::TIME_MASTER_CPU) {
    startCpuThread();
  }
}

PresetSequencer::~PresetSequencer() { stopCpuThread(); }

void PresetSequencer::prepareFirstStep() {
  float playStandaloneTime = 0.0f;
  bool playStandaloneParameters = false;
  // Read parameter changes prior to first preset change
  while (mSteps.size() > 0 && mSteps.front().type == PARAMETER) {
    mParameterList.push(mSteps.front());
    std::cout << "queued " << mSteps.front().presetName << ":"
              << mSteps.front().params[0] << std::endl;
    playStandaloneTime += mSteps.front().waitTime;
    mSteps.pop();
    playStandaloneParameters = true;
  }

  Step step = mSteps.front();
  if (step.type == PRESET && !playStandaloneParameters) {
    if (mPresetHandler) {
      mPresetHandler->setMorphTime(step.morphTime);
      mPresetHandler->recallPreset(step.presetName);
      //                std::cout << "recalling " << step.presetName <<
      //                std::endl;
    } else {
      std::cerr << "No preset handler registered with PresetSequencer. "
                   "Ignoring preset change."
                << std::endl;
    }
    mSteps.pop();
    // Read parameter changes between this and the next preset change.
    assert(mParameterList.empty());
    //    while (!mParameterList.empty()) {
    //      mParameterList.pop();
    //    }
    while (mSteps.size() > 0 && mSteps.front().type == PARAMETER) {
      mParameterList.push(mSteps.front());
      mSteps.pop();
    }
    //    float totalWaitTime = step.morphTime + step.waitTime;
    //    mTargetTime += std::chrono::microseconds(
    //        (int)(totalWaitTime * 1.0e6 - (mGranularity * 1.5 * 1.0e3)));
  }
  if (mParameterList.size() > 0) {
    mParamTargetTime = mParameterList.front().waitTime;
    //            std::cout << "setting param wait " <<
    //            mParameterList.front().waitTime << std::endl;
  }
  mTargetTime = playStandaloneTime;
}

void PresetSequencer::playSequence(std::string sequenceName, double timeScale) {
  stopSequence();
  mSequenceLock.lock();

  if (sequenceName.size() > 0) {
    std::queue<Step> steps = loadSequence(sequenceName, timeScale);
    mSteps = steps;
  }

  // Initialize counters
  //  mSequenceStart = std::chrono::high_resolution_clock::now();
  mCurrentTime = 0.0f;
  mLastTimeUpdate = 0.0f;

  prepareFirstStep();

  if (mTimeMasterMode == TimeMasterMode::TIME_MASTER_CPU) {
    {
      std::unique_lock<std::mutex> lk(mPlayWaitLock);
      mStartingRun = true;
      mPlayWaitVariable.notify_one();
    }
    mSequenceLock.unlock();
    {
      std::unique_lock<std::mutex> lk(mPlayWaitLock);
      mPlayWaitVariable.wait(lk, [&] { return mRunning; });
    }

  } else {
    mSequenceLock.unlock();
    mRunning = true;
  }

  //	std::thread::id seq_thread_id = mSequencerThread->get_id();
  //	std::cout << "Preset Sequencer thread id: " << std::hex << seq_thread_id
  //<< std::endl;
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
    // Is waiting on this lock reliable? Will it hang?
    mPlayWaitLock.lock();  // This waits for the sequencer thread function to
                           // wait on the condition variable.
    mPlayWaitLock.unlock();
  }
}

void PresetSequencer::setTime(double time) {
  if (running()) {
    mTimeRequest = time;
  } else {
    mSteps = mMostRecentSequence;
    double currentTime = 0.0;
    auto sequencer = this;
    auto timeRequest = time;
    if (mSteps.size() > 0) {
      Step step = sequencer->mSteps.front();
      std::string previousPreset = mSteps.front().presetName;
      while (currentTime < timeRequest && sequencer->mSteps.size() > 0) {
        step = sequencer->mSteps.front();
        currentTime += step.morphTime + step.waitTime;
        //                std::cout << "Skipping: " << step.presetName << " " <<
        //                step.morphTime << ":" << step.waitTime << std::endl;
        if (currentTime < timeRequest) {
          previousPreset = sequencer->mSteps.front().presetName;
        }
        sequencer->mSteps.pop();
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
    mSteps = mMostRecentSequence;
    setTime(0.0);
    mSequenceLock.unlock();
  } else {  // If lock can't be acquired, request time change
    setTime(0.0);
  }
}

bool PresetSequencer::archiveSequence(std::string sequenceName,
                                      bool overwrite) {
  bool ok = true;
  std::string fullPath = buildFullPath(sequenceName) + "_archive";
  if (mPresetHandler == nullptr) {
    std::cerr
        << "A Preset Handler must be registered to store sequences. Aborting."
        << std::endl;
    return false;
  }
  if (overwrite) {
    if (File::isDirectory(fullPath)) {
      if (!Dir::removeRecursively(fullPath)) {
        std::cout << "Error removing directory: " << fullPath
                  << " aborting sequence archiving." << std::endl;
        return false;
      }
    } else {
      if (File::remove(fullPath) != 0) {
        std::cout << "Error removing file: " << fullPath
                  << " aborting sequence archiving." << std::endl;
        return false;
      }
    }
    if (!Dir::make(fullPath)) {
      std::cout << "Error creating sequence archive directory " << fullPath
                << std::endl;
      return false;
    }
  } else {
    int counter = 0;
    while (File::isDirectory(fullPath)) {
      std::string newName = sequenceName + "_" + std::to_string(counter++);
      fullPath = buildFullPath(newName) + "_archive";
      if (counter == 0) {  // We've wrapped and run out of names...
        std::cout << "Out of names for sequence archive." << std::endl;
        return false;
      }
    }
    if (!Dir::make(fullPath)) {
      std::cout << "Error creating sequence archive directory " << fullPath
                << std::endl;
      return false;
    }
  }
  std::queue<Step> steps = loadSequence(sequenceName);
  while (steps.size() > 0) {
    Step &step = steps.front();
    std::string presetFilename =
        mPresetHandler->getCurrentPath() + step.presetName + ".preset";
    if (!File::copy(presetFilename, fullPath)) {
      std::cout << "Error copying preset " << presetFilename
                << " when archiving." << std::endl;
      ok = false;
    }
    steps.pop();
  }
  if (!File::copy(buildFullPath(sequenceName), fullPath)) {
    std::cout << "Error copying sequence " << sequenceName << " when archiving."
              << std::endl;
    ok = false;
  }

  return ok;
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
      if (sequencer->mBeginCallbackEnabled &&
          sequencer->mBeginCallback != nullptr) {
        sequencer->mBeginCallback(sequencer, sequencer->mBeginCallbackData);
      }
      sequencer->mPlayWaitVariable.wait(
          lk, [&] { return sequencer->mStartingRun; });
      //      sequencer->mRunning = true;
      //      sequencer->mStartingRun = false;
    }
    sequencer->mRunning = true;
    sequencer->mPlayWaitVariable.notify_one();
    // to have the calling function know play has started
    // and callbacks have been called.
    //    sequencer->mSequenceLock.lock();

    while (sequencer->running()) {
      //      auto now = std::chrono::high_resolution_clock::now();
      //      double currentTime =
      //          1.0e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(
      //                       now - sequencer->mSequenceStart)
      //                       .count();
      sequencer->stepSequencer(sequencer->mGranularity / 1000.0);
      std::this_thread::sleep_for(
          std::chrono::milliseconds(sequencer->mGranularity));
    }
    if (sequencer->mPresetHandler) {
      std::this_thread::sleep_for(std::chrono::milliseconds(
          100));  // Give a little time to process any pending preset changes.
      sequencer->mPresetHandler->stopMorph();
    }
    //	std::cout << "Sequence finished." << std::endl;
    sequencer->mRunning = false;
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

std::queue<PresetSequencer::Step> PresetSequencer::loadSequence(
    std::string sequenceName, double timeScale) {
  std::queue<Step> steps;
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
      steps.push(step);
      //			 std::cout << name  << ":" << delta << ":" <<
      // duration << std::endl;
    } else if (name.size() > 0 && name[0] == '+') {
      Step step;
      step.type = PARAMETER;
      step.presetName = delta;
      step.waitTime = std::stof(name.substr(1)) * timeScale;
      step.params = {float(std::stod(duration) * timeScale)};
      steps.push(step);
      // std::cout << name  << ":" << delta << ":" << duration << std::endl;
    } else if (name.size() > 0 && name[0] != '#' && name[0] != '\r') {
      Step step;
      step.type = PRESET;
      step.presetName = name;
      step.morphTime = std::stof(delta) * timeScale;
      step.waitTime = std::stof(duration) * timeScale;
      steps.push(step);
      // std::cout << name  << ":" << delta << ":" << duration << std::endl;
    }
  }
  if (f.bad()) {
    std::cout << "Error reading:" << sequenceName << std::endl;
  }

  if (!steps.empty()) {
    mCurrentSequence = sequenceName;
    mMostRecentSequence = steps;
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
  float duration = 0.0f;
  std::queue<Step> steps;
  if (sequenceName == mCurrentSequence) {
    steps = mMostRecentSequence;
  } else {
    steps = loadSequence(sequenceName);
  }
  while (steps.size() > 0) {
    const Step &step = steps.front();
    duration += step.morphTime + step.waitTime;
    steps.pop();
  }
  return duration;
}

void PresetSequencer::clearSteps() {
  stopSequence();
  mSequenceLock.lock();
  while (!mSteps.empty()) {
    mSteps.pop();
  }
  mSequenceLock.unlock();
}

void PresetSequencer::appendStep(PresetSequencer::Step &newStep) {
  mSequenceLock.lock();
  mSteps.push(newStep);
  mSequenceLock.unlock();
}

void PresetSequencer::setTimeMaster(TimeMasterMode masterMode) {
  mTimeMasterMode = masterMode;
}

void PresetSequencer::processTimeChangeRequest() {
  // Process time change request
  double timeRequest = double(mTimeRequest.exchange(-1.0f));
  if (timeRequest > 0.0) {
    std::cout << "Requested " << timeRequest << std::endl;
    // Bring back previous steps
    mSteps = mMostRecentSequence;
    mCurrentTime = 0.0;
    //      mSequenceStart = std::chrono::high_resolution_clock::now();
    //                paramTargetTime = now;
    //      mTargetTime = mCurrentTime;
    if (mCurrentTime < timeRequest) {
      std::string previousPreset = mSteps.front().presetName;
      while (mCurrentTime < timeRequest && mSteps.size() > 0) {
        Step step = mSteps.front();
        if (step.type == PRESET) {
          mCurrentTime += step.morphTime + step.waitTime;
          //            std::cout << "Skipping: " << step.presetName << " "
          //                      << step.morphTime << ":" << step.waitTime <<
          //                      std::endl;
          if (mCurrentTime < timeRequest) {
            previousPreset = mSteps.front().presetName;
          }
        }
        mSteps.pop();
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
    // Parameter changes until next preset change have been stored in
    // mParameterList
    if (mParameterList.size() > 0) {
      while (mParamTargetTime <= mCurrentTime) {
        std::string name = mParameterList.front().presetName;
        for (auto *param : mParameters) {
          if (param->getFullAddress() == name) {
            param->fromFloat(mParameterList.front().params[0]);
          }
        }
        mParameterList.pop();
        if (mParameterList.size() > 0) {
          mParamTargetTime += mParameterList.front().waitTime;
        }
      }
    }

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

    while (mTargetTime < mCurrentTime) {
      // Reached target time. Process step
      if (mSteps.size() > 0) {
        Step step = mSteps.front();
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
          mSteps.pop();
          while (mSteps.size() > 0 && mSteps.front().type == PARAMETER) {
            mParameterList.push(mSteps.front());
            std::cout << "queued " << mSteps.front().presetName << ":"
                      << mSteps.front().params[0] << std::endl;
            mSteps.pop();
          }
          mTargetTime += step.morphTime + step.waitTime;
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
    if (mTargetTime < mCurrentTime) {
      mRunning = false;
    }
    mSequenceLock.unlock();
  }
}
// else {
//  if (mRunning) {
//    // If still running, we have reached end of sequence.
//    if (mEndCallback) {
//      mEndCallback(true, this, mEndCallbackData);
//    }
//    mRunning = false;
//  }
//}
//}

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
  mSequencerActive = false;
  stopSequence(false);
  if (mPresetHandler) {
    mPresetHandler->stopMorph();
  }
  this->enableBeginCallback(
      false);           // To vaoid triggering callback on thread wake up
  mStartingRun = true;  // The conditional variable will only stop waiting if
                        // this is true
  this->mPlayWaitVariable.notify_all();
  if (mSequencerThread) {
    mSequencerThread->join();
    mSequencerThread = nullptr;
  }
}

// SequenceServer
// ----------------------------------------------------------------

SequenceServer::SequenceServer(std::string oscAddress, int oscPort)
    : mServer(nullptr),
      mRecorder(nullptr),
      // mParamServer(nullptr),
      mOSCpath("/sequence") {
  mServer = new osc::Recv(oscPort, oscAddress.c_str(),
                          0.001);  // Is this 1ms wait OK?
  if (mServer) {
    mServer->handler(*this);
    mServer->start();
  } else {
    std::cout << "Error starting OSC server." << std::endl;
  }
}

SequenceServer::SequenceServer(ParameterServer &paramServer)
    : mServer(nullptr),
      // mParamServer(&paramServer),
      mOSCpath("/sequence") {
  paramServer.registerOSCListener(this);
}

SequenceServer::~SequenceServer() {
  //	std::cout << "~SequenceServer()" << std::endl;;
  if (mServer) {
    mServer->stop();
    delete mServer;
    mServer = nullptr;
  }
}

void SequenceServer::onMessage(osc::Message &m) {
  if (m.addressPattern() == mOSCpath + "/last") {
    if (mSequencer && mRecorder) {
      std::cout << "start last recorder sequence "
                << mRecorder->lastSequenceName() << std::endl;
      mSequencer->setHandlerSubDirectory(mRecorder->lastSequenceSubDir());
      mSequencer->playSequence(mRecorder->lastSequenceName());
    } else {
      std::cerr << "SequenceRecorder and PresetSequencer must be registered to "
                   "enable /*/last."
                << std::endl;
    }
  } else {
    for (osc::MessageConsumer *consumer : mConsumers) {
      if (consumer->consumeMessage(m, mOSCpath)) {
        break;
      }
    }
  }
}

SequenceServer &SequenceServer::registerMessageConsumer(
    osc::MessageConsumer &consumer) {
  mConsumers.push_back(&consumer);
  return *this;
}

SequenceServer &SequenceServer::registerRecorder(SequenceRecorder &recorder) {
  mRecorder = &recorder;
  mConsumers.push_back(static_cast<osc::MessageConsumer *>(&recorder));
  return *this;
}

SequenceServer &SequenceServer::registerSequencer(PresetSequencer &sequencer) {
  mSequencer = &sequencer;
  mConsumers.push_back(&sequencer);
  return *this;
}

void SequenceServer::print() {
  if (mServer) {
    std::cout << "Sequence server listening on: " << mServer->address() << ":"
              << mServer->port() << std::endl;
    std::cout << "Communicating on path: " << mOSCpath << std::endl;
  }
  for (auto sender : mOSCSenders) {
    std::cout << sender->address() << ":" << sender->port() << std::endl;
  }
}

void SequenceServer::stopServer() {
  if (mServer) {
    mServer->stop();
    delete mServer;
    mServer = nullptr;
  }
}

void SequenceServer::setAddress(std::string address) { mOSCpath = address; }

std::string SequenceServer::getAddress() { return mOSCpath; }

void SequenceServer::changeCallback(int value, void *sender, void *userData) {
  SequenceServer *server = static_cast<SequenceServer *>(userData);
  // Parameter *parameter = static_cast<Parameter *>(sender);
  server->notifyListeners(server->mOSCpath, value);
}
