#include "al/scene/al_DynamicScene.hpp"

#include "al/graphics/al_Shapes.hpp"

#include <algorithm>

using namespace std;
using namespace al;

ThreadPool::ThreadPool(unsigned int n) : busy() {
  for (unsigned int i = 0; i < n; ++i) {
    workers.emplace_back(std::bind(&ThreadPool::thread_proc, this));
  }
}

ThreadPool::~ThreadPool() { stopThreads(); }

void ThreadPool::thread_proc() {
  while (!stop) {
    std::unique_lock<std::mutex> latch(queue_mutex);
    cv_task.wait(latch, [this]() { return stop || !tasks.empty(); });
    if (!tasks.empty()) {
      // got work. set busy.
      ++busy;

      // pull from queue
      auto f = tasks.front();
      tasks.pop_front();

      // release lock. run async
      latch.unlock();

      // run function outside context
      f.first(f.second);

      latch.lock();
      --busy;
      cv_finished.notify_one();
    }
  }
}

// waits until the queue is empty.

void ThreadPool::waitForProcessingDone() {
  std::unique_lock<std::mutex> lock(queue_mutex);
  cv_finished.wait(lock, [this]() { return tasks.empty() && (busy == 0); });
}

void ThreadPool::stopThreads() {
  // set stop-condition
  std::unique_lock<std::mutex> latch(queue_mutex);
  stop = true;
  cv_task.notify_all();
  latch.unlock();
  // all threads terminate, then we're done.
  for (auto &t : workers)
    t.join();
}

// ------------------------------------------------

DynamicScene::DynamicScene(int threadPoolSize, TimeMasterMode masterMode)
    : PolySynth(masterMode) {
  Speakers sl = StereoSpeakerLayout(); // Stereo by default
  setSpatializer<StereoPanner>(sl);
  if (threadPoolSize > 0) {
    mWorkerThreads = std::make_unique<ThreadPool>(threadPoolSize);
  }
  for (int i = 0; i < threadPoolSize; i++) {
    mAudioThreads.push_back(
        std::thread(DynamicScene::audioThreadFunc, this, i));
    mThreadMap[i] = std::vector<int>();
    mThreadMap[i].reserve(16);
  }

  addSphere(mWorldMarker);
  mWorldMarker.primitive(Mesh::LINES);
  mWorldMarker.vertex(0, 0, 0);
  mWorldMarker.index(mWorldMarker.vertices().size() - 1);
  mWorldMarker.vertex(0, 3, 0);
  mWorldMarker.index(mWorldMarker.vertices().size() - 1);
  mWorldMarker.vertex(0, 0, 0);
  mWorldMarker.index(mWorldMarker.vertices().size() - 1);
  mWorldMarker.vertex(3, 0, 0);
  mWorldMarker.index(mWorldMarker.vertices().size() - 1);
  mWorldMarker.vertex(0, 0, 0);
  mWorldMarker.index(mWorldMarker.vertices().size() - 1);
  mWorldMarker.vertex(0, 0, 3);
  mWorldMarker.index(mWorldMarker.vertices().size() - 1);
}

DynamicScene::~DynamicScene() {
  stopAudioThreads();

  if (mWorkerThreads) {
    mWorkerThreads->waitForProcessingDone();
  }
  cleanup();
}

void DynamicScene::prepare(AudioIOData &io) {
  internalAudioIO.framesPerBuffer(io.framesPerBuffer());
  internalAudioIO.channelsIn(mVoiceMaxInputChannels);
  internalAudioIO.channelsOut(mVoiceMaxOutputChannels);
  internalAudioIO.channelsBus(mVoiceBusChannels);
  if ((int)io.channelsBus() < mVoiceBusChannels) {
    std::cout << "WARNING: You don't have enough buses in AudioIO object. This "
                 "is likely to crash."
              << std::endl;
  }
  mThreadedAudioData.resize(mAudioThreads.size());
  for (auto &threadio : mThreadedAudioData) {
    threadio.framesPerBuffer(io.framesPerBuffer());
    threadio.channelsIn(mVoiceMaxInputChannels);
    threadio.channelsOut(mVoiceMaxOutputChannels);
    threadio.channelsBus(mVoiceBusChannels);
  }
  m_internalAudioConfigured = true;
}

void DynamicScene::render(Graphics &g) {
  if (mDrawWorldMarker) {
    g.color(0.7);
    g.draw(mWorldMarker);
  }

  if (mMasterMode == TimeMasterMode::TIME_MASTER_GRAPHICS) {
    processVoices();
    // Turn off voices
    processVoiceTurnOff();
  }
  std::unique_lock<std::mutex> lk(mGraphicsLock);
  std::vector<PositionedVoice *> voices;
  voices.reserve(128);
  auto voice = mActiveVoices;
  while (voice) {
    voices.push_back((PositionedVoice *)voice);
    voice = voice->next;
  }
  if (mSortDrawingByDistance) {
    // FIXME this is crashing in some undetermined cases.
    // For now a working but inefficient way of sorting
    auto viewPos = mListenerPose.pos();
    std::sort(voices.begin(), voices.end(),
              [&](PositionedVoice *a, PositionedVoice *b) -> bool {
                auto distA = (a->pose().pos() - viewPos).mag();
                auto distB = (b->pose().pos() - viewPos).mag();
                return distA >= distB;
              });
  }
  for (auto voice : voices) {
    // TODO implement offset?
    if (voice->active()) {
      g.pushMatrix();
      if (dynamic_cast<PositionedVoice *>(voice)) {
        PositionedVoice *posVoice = static_cast<PositionedVoice *>(voice);
        posVoice->preProcess(g);
        posVoice->applyTransformations(g);
      }
      voice->onProcess(g);
      g.popMatrix();
    }
  }
  if (mMasterMode == TimeMasterMode::TIME_MASTER_GRAPHICS) {
    processInactiveVoices();
  }
}

void DynamicScene::render(AudioIOData &io) {
  if (!m_internalAudioConfigured) {
    prepare(io);
  }
  assert(mSpatializer && "ERROR: call setSpatializer before starting audio");
  io.frame(0);
  mSpatializer->prepare(io);
  if (mMasterMode == TimeMasterMode::TIME_MASTER_AUDIO) {
    processVoices();
    // Turn off voices
    processVoiceTurnOff();
  }
  io.zeroBus();

  auto *voice = mActiveVoices;
  int fpb = internalAudioIO.framesPerBuffer();
  if (mAudioThreads.size() == 0 ||
      !mThreadedAudio) { // Not using worker threads
    // Render active voices
    while (voice) {
      if (voice->active()) {
        int offset = voice->getStartOffsetFrames(fpb);
        if (offset < fpb) {
          // io.frame(offset);
          int endOffsetFrames = voice->getEndOffsetFrames(fpb);
          if (endOffsetFrames > 0 && endOffsetFrames <= fpb) {
            voice->triggerOff(endOffsetFrames);
          }
          internalAudioIO.zeroOut();
          internalAudioIO.zeroBus();
          internalAudioIO.frame(offset);
          voice->onProcess(internalAudioIO);
          Vec3d listeningDir;
          vector<Vec3f> posOffsets;
          if (dynamic_cast<PositionedVoice *>(voice)) {
            PositionedVoice *posVoice = static_cast<PositionedVoice *>(voice);
            Vec3d direction = posVoice->pose().vec() - mListenerPose.vec();

            // Rotate vector according to listener-rotation
            Quatd srcRot = mListenerPose.quat();
            listeningDir = srcRot.rotate(direction);
            posOffsets = posVoice->audioOutOffsets();
            assert(posOffsets.size() == 0 ||
                   posOffsets.size() == posVoice->numOutChannels());
            if (posVoice->useDistanceAttenuation()) {
              float distance = listeningDir.mag();
              float atten = mDistAtten.attenuation(distance);
              internalAudioIO.frame(0);
              float *buf = internalAudioIO.outBuffer(0);

              while (internalAudioIO()) {
                *buf = *buf * atten;
                buf++;
              }
            }
          } else {
            listeningDir = mListenerPose;
          }
          if (mBusRoutingCallback) {
            // First call callback to route signals to internal buses
            internalAudioIO.frame(offset);
            Pose listeningPose = listeningDir;
            (*mBusRoutingCallback)(internalAudioIO, listeningPose);
            io.frame(offset);
            internalAudioIO.frame(offset);
            // Then gather all the internal buses into the master AudioIO buses
            while (io() && internalAudioIO()) {
              for (int i = 0; i < mVoiceBusChannels; i++) {
                io.bus(i) += internalAudioIO.bus(i);
              }
            }
          }
          for (unsigned int i = 0; i < voice->numOutChannels(); i++) {
            io.frame(offset);
            internalAudioIO.frame(offset);
            Pose offsetPose = listeningDir;
            if (posOffsets.size() > 0) {
              // Is there need to rotate the position according to the quat()?
              // It would only really be useful if the source has a direction
              // dependent dispersion model...
              offsetPose.vec() += posOffsets[i];
            }
            mSpatializer->renderBuffer(io, offsetPose,
                                       internalAudioIO.outBuffer(i), fpb);
          }
        }
      }
      voice = voice->next;
    }
  } else { // Process Audio Threaded
    mAudioBusy = 0;
    for (auto &tmap : mThreadMap) {
      tmap.second.resize(0);
    }
    unsigned int counter = 0;
    while (voice) {
      if (voice->active()) {
        mThreadMap[counter++].push_back(voice->id());
        if (counter > mThreadMap.size()) {
          counter = 0;
        }
      }
      voice = voice->next;
    }
    externalAudioIO = &io;
    mThreadTrigger.notify_all();
    std::unique_lock<std::mutex> lk(mThreadTriggerLock);
    mAudioThreadDone.wait(lk, [this]() { return mAudioBusy == 0; });
  }
  mSpatializer->finalize(io);
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

void DynamicScene::update(double dt) {
  if (mMasterMode == TimeMasterMode::TIME_MASTER_UPDATE) {
    processVoices();
    // Turn off voices
    processVoiceTurnOff();
  }

  if (!mWorkerThreads || !mThreadedUpdate) { // Not using worker threads
    auto *voice = mActiveVoices;
    while (voice) {
      if (voice->active()) {
        voice->update(dt);
      }
      voice = voice->next;
    }
  } else { // Using worker threads

    auto *voice = mActiveVoices;
    while (voice) {
      if (voice->active()) {
        UpdateThreadFuncData data{voice, dt};
        mWorkerThreads->enqueue(DynamicScene::updateThreadFunc, data);
      }
      voice = voice->next;
    }
    mWorkerThreads->waitForProcessingDone();
  }
  // Update
  if (mMasterMode == TimeMasterMode::TIME_MASTER_UPDATE) {
    processInactiveVoices();
  }
}

void DynamicScene::print(ostream &stream) {
  stream << "Audio Distance Attenuation:";
  const char *s = nullptr;
#define PROCESS_VAL(p)                                                         \
  case (p):                                                                    \
    s = #p;                                                                    \
    break;
  switch (mDistAtten.law()) {
    PROCESS_VAL(ATTEN_NONE);
    PROCESS_VAL(ATTEN_LINEAR);
    PROCESS_VAL(ATTEN_INVERSE);
    PROCESS_VAL(ATTEN_INVERSE_SQUARE);
  }
#undef PROCESS_VAL
  stream << "Law: " << s << std::endl;
  stream << "Near clip: " << mDistAtten.nearClip()
         << "   Far clip: " << mDistAtten.farClip() << std::endl;
  stream << "Far bias: " << mDistAtten.farBias() << std::endl;

  mSpatializer->print(stream);

  PolySynth::print(stream);
}

void DynamicScene::sortDrawingByDistance(bool sort) {
  mSortDrawingByDistance = sort;
}

void DynamicScene::updateThreadFunc(UpdateThreadFuncData data) {
  //        UpdateThreadFuncData *data = (UpdateThreadFuncData *) ud;
  //        std::cout << "eq " << data.voice << "  " << data.dt << std::endl;
  SynthVoice *voice = data.voice;
  double &dt = data.dt;
  voice->update(dt);
}

void DynamicScene::audioThreadFunc(DynamicScene *scene, int id) {
  while (scene->mSynthRunning) {
    std::unique_lock<std::mutex> lk(scene->mThreadTriggerLock);
    scene->mThreadTrigger.wait(lk);
    scene->mAudioBusy++;

    vector<int> &idsToProcess = scene->mThreadMap[id];
    AudioIOData &internalAudioIO = scene->mThreadedAudioData[id];
    AudioIOData &io = *scene->externalAudioIO;
    unsigned int fpb = internalAudioIO.framesPerBuffer();
    SynthVoice *voice = scene->mActiveVoices;
    while (voice) {
      //                if (voice->active()) { // No need to check as only
      //                active voices are put in mThreadMap
      if (find(idsToProcess.begin(), idsToProcess.end(), voice->id()) !=
          idsToProcess.end()) { // voice has been assigned to this thread
        unsigned int offset = voice->getStartOffsetFrames(fpb);
        if (offset < fpb) {
          io.frame(offset);
          unsigned int endOffsetFrames = voice->getEndOffsetFrames(fpb);
          if (endOffsetFrames > 0 && endOffsetFrames <= fpb) {
            voice->triggerOff(endOffsetFrames);
          }
          internalAudioIO.zeroOut();
          internalAudioIO.zeroBus();
          internalAudioIO.frame(offset);
          voice->onProcess(internalAudioIO);
          Vec3d listeningDir;
          vector<Vec3f> posOffsets;
          if (dynamic_cast<PositionedVoice *>(voice)) {
            PositionedVoice *posVoice = static_cast<PositionedVoice *>(voice);
            Vec3d direction =
                posVoice->pose().vec() - scene->mListenerPose.vec();

            // Rotate vector according to listener-rotation
            Quatd srcRot = scene->mListenerPose.quat();
            listeningDir = srcRot.rotate(direction);
            posOffsets = posVoice->audioOutOffsets();
            assert(posOffsets.size() == 0 ||
                   posOffsets.size() == posVoice->numOutChannels());
            if (posVoice->useDistanceAttenuation()) {
              float distance = scene->mListenerPose.vec().mag();
              float atten = scene->mDistAtten.attenuation(distance);
              internalAudioIO.frame(0);
              float *buf = internalAudioIO.outBuffer(0);

              while (internalAudioIO()) {
                *buf = *buf * atten;
                buf++;
              }
            }
          } else {
            listeningDir = scene->mListenerPose;
            // FIXME what should we do here if voice not a PositionedVoice?
          }
          scene->mSpatializerLock.lock();
          if (scene->mBusRoutingCallback) {
            // First call callback to route signals to internal buses
            internalAudioIO.frame(offset);
            (*scene->mBusRoutingCallback)(internalAudioIO,
                                          scene->mListenerPose);
            io.frame(offset);
            internalAudioIO.frame(offset);
            // Then gather all the internal buses into the master AudioIO buses
            while (io() && internalAudioIO()) {
              for (int i = 0; i < scene->mVoiceBusChannels; i++) {
                io.bus(i) += internalAudioIO.bus(i);
              }
            }
          }
          for (unsigned int i = 0; i < voice->numOutChannels(); i++) {
            io.frame(offset);
            internalAudioIO.frame(offset);
            Pose offsetPose = scene->mListenerPose;
            if (posOffsets.size() > 0) {
              offsetPose.vec() += posOffsets[i];
            }
            scene->mSpatializer->renderBuffer(
                io, offsetPose, internalAudioIO.outBuffer(i), fpb);
          }
          scene->mSpatializerLock.unlock();
        }
      }
      voice = voice->next;
    }
    scene->mAudioBusy--;
    scene->mAudioThreadDone.notify_one();
  }
  //  std::cout << "Audio thread " << id << " done" << std::endl;
}

bool PositionedVoice::setTriggerParams(float *pFields, int numFields) {
  bool ok = SynthVoice::setTriggerParams(pFields, numFields);
  if (numFields ==
      (int)mTriggerParams.size() +
          8) { // If seven extra, it means pose and size are there too
    pFields += mTriggerParams.size();
    double x = *pFields++;
    double y = *pFields++;
    double z = *pFields++;
    double qw = *pFields++;
    double qx = *pFields++;
    double qy = *pFields++;
    double qz = *pFields++;
    mPose.set({Vec3d(x, y, z), Quatd(qw, qx, qy, qz)});
    mSize.set(*pFields);
  } else {
    ok = false;
  }
  return ok;
}

bool PositionedVoice::setTriggerParams(std::vector<float> &pFields,
                                       bool noCalls) {
  bool ok = SynthVoice::setTriggerParams(pFields, noCalls);
  if (pFields.size() ==
      mTriggerParams.size() +
          8) { // If seven extra, it means pose and size are there too
    size_t index = mTriggerParams.size();
    double x = pFields[index++];
    double y = pFields[index++];
    double z = pFields[index++];
    double qw = pFields[index++];
    double qx = pFields[index++];
    double qy = pFields[index++];
    double qz = pFields[index++];
    if (noCalls) {
      mPose.setNoCalls({Vec3d(x, y, z), Quatd(qw, qx, qy, qz)});
      mSize.setNoCalls(pFields[index++]);
    } else {
      mPose.set({Vec3d(x, y, z), Quatd(qw, qx, qy, qz)});
      mSize.set(pFields[index++]);
    }
  } else {
    //            std::cout << "Not setting position for voice" << std::endl;
    ok = false;
  }
  return ok;
  //  return setTriggerParams(pFields.data(), pFields.size(), noCalls);
}

bool PositionedVoice::setTriggerParams(std::vector<VariantValue> pFields,
                                       bool noCalls) {
  bool ok = SynthVoice::setTriggerParams(pFields);
  if (pFields.size() ==
      mTriggerParams.size() +
          8) { // If seven extra, it means pose and size are there too
    size_t index = mTriggerParams.size();
    double x = pFields[index++].get<float>();
    double y = pFields[index++].get<float>();
    double z = pFields[index++].get<float>();
    double qw = pFields[index++].get<float>();
    double qx = pFields[index++].get<float>();
    double qy = pFields[index++].get<float>();
    double qz = pFields[index++].get<float>();
    if (noCalls) {
      mPose.setNoCalls({Vec3d(x, y, z), Quatd(qw, qx, qy, qz)});
      mSize.setNoCalls(pFields[index++].get<float>());
    } else {
      mPose.set({Vec3d(x, y, z), Quatd(qw, qx, qy, qz)});
      mSize.set(pFields[index++].get<float>());
    }
  } else {
    //            std::cout << "Not setting position for voice" << std::endl;
    ok = false;
  }
  return ok;
}

void PositionedVoice::applyTransformations(Graphics &g) {
  auto pose = mPose.get();
  g.translate(pose.x(), pose.y(), pose.z());
  g.rotate(pose.quat());
  g.scale(size());
}
