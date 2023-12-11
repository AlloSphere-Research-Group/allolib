#include "al/scene/al_PolySynth.hpp"

#include <memory>

using namespace al;

int al::asciiToIndex(int asciiKey, int offset) {
  switch (asciiKey) {
  case '1':
    return offset + 0;
  case '2':
    return offset + 1;
  case '3':
    return offset + 2;
  case '4':
    return offset + 3;
  case '5':
    return offset + 4;
  case '6':
    return offset + 5;
  case '7':
    return offset + 6;
  case '8':
    return offset + 7;
  case '9':
    return offset + 8;
  case '0':
    return offset + 9;

  case 'q':
    return offset + 10;
  case 'w':
    return offset + 11;
  case 'e':
    return offset + 12;
  case 'r':
    return offset + 13;
  case 't':
    return offset + 14;
  case 'y':
    return offset + 15;
  case 'u':
    return offset + 16;
  case 'i':
    return offset + 17;
  case 'o':
    return offset + 18;
  case 'p':
    return offset + 19;
  case 'a':
    return offset + 20;

  case 's':
    return offset + 21;
  case 'd':
    return offset + 22;
  case 'f':
    return offset + 23;
  case 'g':
    return offset + 24;
  case 'h':
    return offset + 25;
  case 'j':
    return offset + 26;
  case 'k':
    return offset + 27;
  case 'l':
    return offset + 28;
  case ';':
    return offset + 29;

  case 'z':
    return offset + 30;
  case 'x':
    return offset + 31;
  case 'c':
    return offset + 32;
  case 'v':
    return offset + 33;
  case 'b':
    return offset + 34;
  case 'n':
    return offset + 35;
  case 'm':
    return offset + 36;
  case ',':
    return offset + 37;
  case '.':
    return offset + 38;
  case '/':
    return offset + 39;
  }
  return 0;
}

int al::asciiToMIDI(int asciiKey, int offset) {
  switch (asciiKey) {
  //	case '1': return offset + 0;
  case '2':
    return offset + 73;
  case '3':
    return offset + 75;
  //	case '4': return offset + 3;
  case '5':
    return offset + 78;
  case '6':
    return offset + 80;
  case '7':
    return offset + 82;
  //	case '8': return offset + 7;
  case '9':
    return offset + 85;
  case '0':
    return offset + 87;

  case 'q':
    return offset + 72;
  case 'w':
    return offset + 74;
  case 'e':
    return offset + 76;
  case 'r':
    return offset + 77;
  case 't':
    return offset + 79;
  case 'y':
    return offset + 81;
  case 'u':
    return offset + 83;
  case 'i':
    return offset + 84;
  case 'o':
    return offset + 86;
  case 'p':
    return offset + 88;

  //	case 'a': return offset + 20;
  case 's':
    return offset + 61;
  case 'd':
    return offset + 63;
  //	case 'f': return offset + 23;
  case 'g':
    return offset + 66;
  case 'h':
    return offset + 68;
  case 'j':
    return offset + 70;
  //	case 'k': return offset + 27;
  case 'l':
    return offset + 73;
  case ';':
    return offset + 75;

  case 'z':
    return offset + 60;
  case 'x':
    return offset + 62;
  case 'c':
    return offset + 64;
  case 'v':
    return offset + 65;
  case 'b':
    return offset + 67;
  case 'n':
    return offset + 69;
  case 'm':
    return offset + 71;
  case ',':
    return offset + 72;
  case '.':
    return offset + 74;
  case '/':
    return offset + 76;
  }
  return 0;
}

// ----------------------------

PolySynth::PolySynth(TimeMasterMode masterMode) : mMasterMode(masterMode) {
  if (mMasterMode == TimeMasterMode::TIME_MASTER_CPU) {
    startCpuClockThread();
  }
}

PolySynth::~PolySynth() {
  if (mCpuClockThread) {
    mRunCPUClock = false;
    mCpuClockThread->join();
  }
}

int PolySynth::triggerOn(SynthVoice *voice, int offsetFrames, int id,
                         void *userData) {
  assert(voice);
  if (verbose()) {
    std::cout << "Trigger on ";
    for (auto *param : voice->triggerParameters()) {
      if (strcmp(typeid(*param).name(), typeid(ParameterString).name()) == 0) {
        std::cout << param->getName() << ":"
                  << static_cast<ParameterString *>(param)->get() << " ";

      } else {
        std::cout << param->getName() << ":" << param->toFloat() << " ";
      }
    }
    std::cout << std::endl;
  }
  int thisId = id;
  if (thisId == -1) {
    if (voice->id() >= 0) {
      thisId = voice->id();
    } else {
      thisId = mIdCounter++;
    }
  }
  voice->id(thisId);
  if (userData) {
    voice->userData(userData);
  }
  bool allCallbacksOk = true;
  for (const auto &cbNode : mTriggerOnCallbacks) {
    allCallbacksOk &= cbNode.first(voice, offsetFrames, thisId, cbNode.second);
  }
  if (allCallbacksOk) {
    voice->triggerOn(offsetFrames);
    {
      std::unique_lock<std::mutex> lk(mVoiceToInsertLock);
      voice->next = mVoicesToInsert;
      voice->mActive =
          true; // We need to mark this here to avoid race conditions if
                // active() is checked on separate thread, and the voice
                // removed before it has been triggered.
      mVoicesToInsert = voice;
    }
    return thisId;
  } else {
    return -1;
  }
}

void PolySynth::triggerOff(int id) {
  bool allCallbacksOk = true;
  for (auto cbNode : mTriggerOffCallbacks) {
    allCallbacksOk &= cbNode.first(id, cbNode.second);
  }
  if (allCallbacksOk) {
    mVoiceIdsToTurnOff.write((const char *)&id, sizeof(int));
  }
}

void PolySynth::allNotesOff() { mAllNotesOff = true; }

SynthVoice *PolySynth::getVoice(std::string name, bool forceAlloc) {
  std::unique_lock<std::mutex> lk(
      mFreeVoiceLock); // Only one getVoice() call at a time
  SynthVoice *freeVoice = mFreeVoices;
  SynthVoice *previousVoice = nullptr;
  while (freeVoice) {
    if (verbose()) {
      std::cout << "Comparing  voice '" << demangle(typeid(*freeVoice).name())
                << "' to '" << name << "'" << std::endl;
    }
    if (demangle(typeid(*freeVoice).name()) == name ||
        strncmp(typeid(*freeVoice).name(), name.c_str(), name.size()) == 0) {
      if (previousVoice) {
        previousVoice->next = freeVoice->next;
      } else {
        mFreeVoices = freeVoice->next;
      }
      break;
    }
    previousVoice = freeVoice;
    freeVoice = freeVoice->next;
  }
  if (!freeVoice) { // No free voice in list, so we need to allocate it
                    //  But only allocate if allocation has not been
                    //  disabled
    if (std::find(mNoAllocationList.begin(), mNoAllocationList.end(), name) ==
        mNoAllocationList.end()) {
      // TODO report current polyphony for more informed allocation of
      // polyphony
      freeVoice = allocateVoice(name);
    } else {
      std::cout << "Automatic allocation disabled for voice:" << name
                << std::endl;
    }
  }
  return freeVoice;
}

SynthVoice *PolySynth::getFreeVoice() {
  std::unique_lock<std::mutex> lk(
      mFreeVoiceLock); // Only one getVoice() call at a time
  SynthVoice *freeVoice = mFreeVoices;
  if (freeVoice) {
    mFreeVoices = freeVoice->next;
  }
  return freeVoice;
}

void PolySynth::render(AudioIOData &io) {
  if (!m_internalAudioConfigured) {
    prepare(io);
  }
  if (mMasterMode == TimeMasterMode::TIME_MASTER_AUDIO) {
    processVoices();
    // Turn off voices
    processVoiceTurnOff();
  }

  // Render active voices
  auto *voice = mActiveVoices;
  int fpb = io.framesPerBuffer();
  while (voice) {
    if (voice->active()) {
      int offset = voice->getStartOffsetFrames(fpb);
      if (offset < fpb) {
        int endOffsetFrames = voice->getEndOffsetFrames(fpb);
        if (endOffsetFrames > 0 && endOffsetFrames <= fpb) {
          voice->triggerOff(endOffsetFrames);
        }
        if (m_useInternalAudioIO) {
          internalAudioIO.zeroOut();
          internalAudioIO.zeroBus();
          internalAudioIO.frame(offset);
          voice->onProcess(internalAudioIO);

          if (mBusRoutingCallback) {
            // First call callback to route signals to internal buses
            internalAudioIO.frame(offset);
            Pose p;
            (*mBusRoutingCallback)(internalAudioIO, p);
          }
          // Then gather all the internal buses into the master AudioIO
          // buses
          io.frame(offset);
          internalAudioIO.frame(offset);
          while (io() && internalAudioIO()) {
            for (int i = 0; i < mVoiceMaxOutputChannels; i++) {
              if (mChannelMap.size() > i) {
                io.out(mChannelMap[i]) += internalAudioIO.out(i);
              } else {
                io.out(i) += internalAudioIO.out(i);
              }
            }
            for (int i = 0; i < mVoiceBusChannels; i++) {
              io.bus(i) += internalAudioIO.bus(i);
            }
          }
        }
      } else {
        io.frame(offset);
        voice->onProcess(io);
      }
    }
    voice = voice->next;
  }
  processGain(io);
  // Run post processing callbacks
  for (auto cb : mPostProcessing) {
    io.frame(0);
    cb->onAudioCB(io);
  }
  if (mMasterMode == TimeMasterMode::TIME_MASTER_AUDIO) {
    processInactiveVoices();
  }
}

void PolySynth::render(Graphics &g) {
  if (mMasterMode == TimeMasterMode::TIME_MASTER_GRAPHICS) {
    processVoices();
    // Turn off voices
    processVoiceTurnOff();
  }
  std::unique_lock<std::mutex> lk(mGraphicsLock);
  SynthVoice *voice = mActiveVoices;
  while (voice) {
    // TODO implement offset?
    if (voice->active()) {
      voice->onProcess(g);
    }
    voice = voice->next;
  }
  if (mMasterMode == TimeMasterMode::TIME_MASTER_GRAPHICS) {
    processInactiveVoices();
  }
}

void PolySynth::update(double dt) {
  if (mMasterMode == TimeMasterMode::TIME_MASTER_UPDATE) {
    processVoices();
    // Turn off voices
    processVoiceTurnOff();
  }
  std::unique_lock<std::mutex> lk(mGraphicsLock);
  SynthVoice *voice = mActiveVoices;
  while (voice) {
    if (voice->active()) {
      voice->update(dt);
    }
    voice = voice->next;
  }
  if (mMasterMode == TimeMasterMode::TIME_MASTER_UPDATE) {
    processInactiveVoices();
  }
}

void PolySynth::disableAllocation(std::string name) {
  if (std::find(mNoAllocationList.begin(), mNoAllocationList.end(), name) ==
      mNoAllocationList.end()) {
    mNoAllocationList.push_back(name);
  }
}

void PolySynth::allocatePolyphony(std::string name, int number) {
  std::unique_lock<std::mutex> lk(mFreeVoiceLock);
  // Find last voice and add polyphony there
  SynthVoice *lastVoice = mFreeVoices;
  if (lastVoice) {
    while (lastVoice->next) {
      lastVoice = lastVoice->next;
    }
  } else {
    lastVoice = mFreeVoices = allocateVoice(name);
    number--;
  }
  for (int i = 0; i < number; i++) {
    lastVoice->next = allocateVoice(name);
    lastVoice = lastVoice->next;
  }
}

void PolySynth::insertFreeVoice(SynthVoice *voice) {
  std::unique_lock<std::mutex> lk(mFreeVoiceLock);
  voice->next = mFreeVoices;
  mFreeVoices = voice;
}

bool PolySynth::popFreeVoice(SynthVoice *voice) {
  std::unique_lock<std::mutex> lk(mFreeVoiceLock);
  SynthVoice *lastVoice = mFreeVoices;
  SynthVoice *previousVoice = nullptr;
  while (lastVoice) {
    if (lastVoice == voice) {
      if (previousVoice) {
        previousVoice->next = lastVoice->next;
        voice->next = nullptr;
      } else {
        mFreeVoices = lastVoice->next;
        voice->next = nullptr;
      }
      return true;
    }
    lastVoice = lastVoice->next;
  }
  return false;
}

void PolySynth::setTimeMaster(TimeMasterMode masterMode) {
  mMasterMode = masterMode;
  if (mMasterMode == TimeMasterMode::TIME_MASTER_CPU) {
    mRunCPUClock = true;
    startCpuClockThread();
  } else {
    if (mCpuClockThread) {
      mRunCPUClock = false;
      mCpuClockThread->join();
      mCpuClockThread = nullptr;
    }
  }
}

PolySynth &PolySynth::append(AudioCallback &v) {
  mPostProcessing.push_back(&v);
  return *this;
}

PolySynth &PolySynth::prepend(AudioCallback &v) {
  mPostProcessing.insert(mPostProcessing.begin(), &v);
  return *this;
}

PolySynth &PolySynth::insertBefore(AudioCallback &v,
                                   AudioCallback &beforeThis) {
  std::vector<AudioCallback *>::iterator pos =
      std::find(mPostProcessing.begin(), mPostProcessing.end(), &beforeThis);
  if (pos == mPostProcessing.begin()) {
    prepend(v);
  } else {
    mPostProcessing.insert(--pos, &v);
  }
  return *this;
}

PolySynth &PolySynth::insertAfter(AudioCallback &v, AudioCallback &afterThis) {
  std::vector<AudioCallback *>::iterator pos =
      std::find(mPostProcessing.begin(), mPostProcessing.end(), &afterThis);
  if (pos == mPostProcessing.end()) {
    append(v);
  } else {
    mPostProcessing.insert(pos, &v);
  }
  return *this;
}

PolySynth &PolySynth::remove(AudioCallback &v) {
  // the proper way to do it:
  mPostProcessing.erase(
      std::remove(mPostProcessing.begin(), mPostProcessing.end(), &v),
      mPostProcessing.end());
  return *this;
}

void PolySynth::print(std::ostream &stream) {
  {
    std::unique_lock<std::mutex> lk(mFreeVoiceLock);
    auto voice = mFreeVoices;
    int counter = 0;
    stream << " ---- Free Voices ----" << std::endl;
    while (voice) {
      stream << "Voice " << counter++ << " " << voice->id() << " : "
             << typeid(voice).name() << " " << voice << std::endl;
      voice = voice->next;
    }
  }
  //
  {
    auto *voice = mActiveVoices;
    int counter = 0;
    stream << " ---- Active Voices ----" << std::endl;
    while (voice) {
      stream << "Voice " << counter++ << " " << voice->id() << " : "
             << typeid(voice).name() << " " << voice << std::endl;
      voice = voice->next;
    }
  }
  //
  {
    std::unique_lock<std::mutex> lk(mVoiceToInsertLock);
    auto voice = mVoicesToInsert;
    int counter = 0;
    stream << " ---- Queued Voices ----" << std::endl;
    while (voice) {
      stream << "Voice " << counter++ << " " << voice->id() << " : "
             << typeid(voice).name() << " " << voice << std::endl;
      voice = voice->next;
    }
  }
}

void PolySynth::registerTriggerOnCallback(
    std::function<bool(SynthVoice *, int, int, void *)> cb, void *userData) {
  TriggerOnCallback cbNode(cb, userData);
  mTriggerOnCallbacks.push_back(cbNode);
}

void PolySynth::registerTriggerOffCallback(std::function<bool(int, void *)> cb,
                                           void *userData) {
  TriggerOffCallback cbNode(cb, userData);
  mTriggerOffCallbacks.push_back(cbNode);
}

void PolySynth::registerFreeCallback(std::function<bool(int, void *)> cb,
                                     void *userData) {
  FreeCallback cbNode(cb, userData);
  mFreeCallbacks.push_back(cbNode);
}

SynthVoice *PolySynth::allocateVoice(std::string name) {
  if (mCreators.find(name) != mCreators.end()) {
    if (mVerbose) {
      std::cout << "Allocating (from name) voice of type " << name << "."
                << std::endl;
    }
    SynthVoice *voice = mCreators[name]();
    return voice;
  } else {
    if (mVerbose) {
      std::cout << "Can't allocate voice of type " << name
                << ". Voice not registered and no polyphony." << std::endl;
    }
  }
  return nullptr;
}

void PolySynth::setVoiceMaxOutputChannels(uint16_t channels) {
  mVoiceMaxOutputChannels = channels;
  for (size_t i = 0; i < channels; i++) {
    mChannelMap[i] = i;
  }
}

void PolySynth::setBusRoutingCallback(PolySynth::BusRoutingCallback cb) {
  mBusRoutingCallback = std::make_shared<BusRoutingCallback>(cb);
}

void PolySynth::setChannelMap(std::vector<size_t> channelMap) {
  if (channelMap.size() != mVoiceMaxOutputChannels) {
    std::cerr << "ERROR setting channel map. " << __FUNCTION__
              << " in " __FILE__ << ":" << __LINE__ << std::endl;
    return;
  }
  mChannelMap = channelMap;
}

void PolySynth::startCpuClockThread() {
  if (mVerbose) {
    std::cout << "Starting CPU clock thread" << std::endl;
  }
  if (!mCpuClockThread) {
    mCpuClockThread = std::make_unique<std::thread>([this]() {
      using namespace std::chrono;
      while (mRunCPUClock) {
        high_resolution_clock::time_point startTime =
            high_resolution_clock::now();
        std::chrono::milliseconds waitTime(int(mCpuGranularitySec * 1000));

        high_resolution_clock::time_point futureTime = startTime + waitTime;
        std::this_thread::sleep_until(futureTime);
        // FIXME this will generate some jitter and drift, fix.

        processVoices();
        // Turn off voices
        processVoiceTurnOff();
        processInactiveVoices();
      }
    });
  }
}

void PolySynth::prepare(AudioIOData &io) {
  internalAudioIO.framesPerBuffer(io.framesPerBuffer());
  internalAudioIO.channelsIn(mVoiceMaxInputChannels);
  internalAudioIO.channelsOut(mVoiceMaxOutputChannels);
  internalAudioIO.channelsBus(mVoiceBusChannels);
  if ((int)io.channelsBus() < mVoiceBusChannels) {
    std::cout << "WARNING: You don't have enough buses in AudioIO object. "
                 "This is likely to crash."
              << std::endl;
  }
  //      mThreadedAudioData.resize(mAudioThreads.size());
  //      for (auto &threadio: mThreadedAudioData) {
  //          threadio.framesPerBuffer(io.framesPerBuffer());
  //          threadio.channelsIn(mVoiceMaxInputChannels);
  //          threadio.channelsOut(mVoiceMaxOutputChannels);
  //          threadio.channelsBus(mVoiceBusChannels);
  //      }
  m_internalAudioConfigured = true;
}

void PolySynth::registerAllocateCallback(
    std::function<void(SynthVoice *, void *)> cb, void *userData) {
  AllocationCallback cbNode(cb, userData);
  mAllocationCallbacks.push_back(cbNode);
}
