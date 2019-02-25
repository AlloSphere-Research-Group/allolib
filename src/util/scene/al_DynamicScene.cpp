#include "al/util/scene/al_DynamicScene.hpp"
#include "al/core/graphics/al_Shapes.hpp"

using namespace std;
using namespace al;

ThreadPool::ThreadPool(unsigned int n)
    : busy()
    , stop()
{
    for (unsigned int i=0; i<n; ++i)
        workers.emplace_back(std::bind(&ThreadPool::thread_proc, this));
}


ThreadPool::~ThreadPool()
{
    // set stop-condition
    std::unique_lock<std::mutex> latch(queue_mutex);
    stop = true;
    cv_task.notify_all();
    latch.unlock();

    // all threads terminate, then we're done.
    for (auto& t : workers)
        t.join();
}

void ThreadPool::thread_proc()
{
    while (true)
    {
        std::unique_lock<std::mutex> latch(queue_mutex);
        cv_task.wait(latch, [this](){ return stop || !tasks.empty(); });
        if (!tasks.empty())
        {
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
        } else if (stop)
        {
            break;
        }
    }
}

// waits until the queue is empty.

void ThreadPool::waitFinished()
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    cv_finished.wait(lock, [this](){ return tasks.empty() && (busy == 0); });
}

// ------------------------------------------------

DynamicScene::DynamicScene (int threadPoolSize, TimeMasterMode masterMode)
    : PolySynth(masterMode)
{
    SpeakerLayout sl = StereoSpeakerLayout(); // Stereo by default
    setSpatializer<StereoPanner>(sl);
    if (threadPoolSize > 0) {
        mWorkerThreads = std::make_unique<ThreadPool>(threadPoolSize);
    }
    for (int i = 0; i < threadPoolSize; i++) {
        mAudioThreads.push_back(std::thread(DynamicScene::audioThreadFunc,this, i));
        mThreadMap[i] = std::vector<int>();
        mThreadMap[i].reserve(16);
    }

    addSphere(mWorldMarker);
    mWorldMarker.primitive(Mesh::LINES);
    mWorldMarker.vertex(0,0,0);
    mWorldMarker.index(mWorldMarker.vertices().size() -1);
    mWorldMarker.vertex(0,3,0);
    mWorldMarker.index(mWorldMarker.vertices().size() -1);
    mWorldMarker.vertex(0,0,0);
    mWorldMarker.index(mWorldMarker.vertices().size() -1);
    mWorldMarker.vertex(3,0,0);
    mWorldMarker.index(mWorldMarker.vertices().size() -1);
    mWorldMarker.vertex(0,0,0);
    mWorldMarker.index(mWorldMarker.vertices().size() -1);
    mWorldMarker.vertex(0,0,3);
    mWorldMarker.index(mWorldMarker.vertices().size() -1);
}

DynamicScene::~DynamicScene() {
    mSynthRunning = false;
    mThreadTrigger.notify_all();
    if (mWorkerThreads) {
        mWorkerThreads->waitFinished();
    }
    for (auto &thr : mAudioThreads) {
        thr.join();
    }
    cleanup();
}

void DynamicScene::prepare(AudioIOData &io) {
    internalAudioIO.framesPerBuffer(io.framesPerBuffer());
    internalAudioIO.channelsIn(mVoiceMaxInputChannels);
    internalAudioIO.channelsOut(mVoiceMaxOutputChannels);
    internalAudioIO.channelsBus(mVoiceBusChannels);
    if ((int) io.channelsBus() < mVoiceBusChannels) {
        std::cout << "WARNING: You don't have enough buses in AudioIO object. This is likely to crash." << std::endl;
    }
    mThreadedAudioData.resize(mAudioThreads.size());
    for (auto &threadio: mThreadedAudioData) {
        threadio.framesPerBuffer(io.framesPerBuffer());
        threadio.channelsIn(mVoiceMaxInputChannels);
        threadio.channelsOut(mVoiceMaxOutputChannels);
        threadio.channelsBus(mVoiceBusChannels);
    }
}

void DynamicScene::render(Graphics &g) {
    if (mDrawWorldMarker) {
        g.color(0.7);
        g.draw(mWorldMarker);
    }

    if (mMasterMode == TIME_MASTER_GRAPHICS) {
        processVoices();
        // Turn off voices
        processVoiceTurnOff();
    }
    std::unique_lock<std::mutex> lk(mGraphicsLock);
    SynthVoice *voice = mActiveVoices;
    while (voice) {
        // TODO implement offset?
        if (voice->active()) {
            g.pushMatrix();
            if (dynamic_cast<PositionedVoice *>(voice)) {
                PositionedVoice *posVoice = static_cast<PositionedVoice *>(voice);
                Pose &pose = posVoice->pose();
                g.translate(pose.x(), pose.y(), pose.z());
                g.rotate(pose.quat());
                g.scale(posVoice->size());
            }
            voice->onProcess(g);
            g.popMatrix();
        }
        voice = voice->next;
    }
    if (mMasterMode == TIME_MASTER_GRAPHICS) {
        processInactiveVoices();
    }
}

void DynamicScene::render(AudioIOData &io) {
    assert(mSpatializer && "ERROR: call setSpatializer before starting audio");
    io.frame(0);
    mSpatializer->prepare(io);
    if (mMasterMode == TIME_MASTER_AUDIO) {
        processVoices();
        // Turn off voices
        processVoiceTurnOff();
    }

    auto voice = mActiveVoices;
    int fpb = internalAudioIO.framesPerBuffer();
    if (mAudioThreads.size() == 0 || !mThreadedAudio) { // Not using worker threads
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

                        //Rotate vector according to listener-rotation
                        Quatd srcRot = mListenerPose.quat();
                        listeningDir = srcRot.rotate(direction);
                        posOffsets = posVoice->audioOutOffsets();
                        assert(posOffsets.size() == 0 || posOffsets.size() == posVoice->numOutChannels());
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
                            // It would only really be useful if the source has a direction dependent
                            // dispersion model...
                            offsetPose.vec() += posOffsets[i];
                        }
                        mSpatializer->renderBuffer(io, offsetPose, internalAudioIO.outBuffer(i), fpb);

                    }
                }
            }
            voice = voice->next;
        }
    } else { // Process Audio Threaded
        mAudioBusy = 0;
        for (auto& tmap: mThreadMap) {
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
        mAudioThreadDone.wait(lk, [this](){ return mAudioBusy == 0; });
    }
    mSpatializer->finalize(io);
    processGain(io);

    // Run post processing callbacks
    for (auto cb: mPostProcessing) {
        io.frame(0);
        cb->onAudioCB(io);
    }
    if (mMasterMode == TIME_MASTER_AUDIO) {
        processInactiveVoices();
    }
}

void DynamicScene::update(double dt) {
    if (mMasterMode == TIME_MASTER_ASYNC) {
        processVoices();
        // Turn off voices
        processVoiceTurnOff();
    }

    if (!mWorkerThreads || !mThreadedUpdate) { // Not using worker threads
        auto voice = mActiveVoices;
        while (voice) {
            if (voice->active()) {
                voice->update(dt);
            }
            voice = voice->next;
        }
    } else { // Using worker threads

        auto voice = mActiveVoices;
        while (voice) {
            if (voice->active()) {
                UpdateThreadFuncData data {voice, dt};
                mWorkerThreads->enqueue(DynamicScene::updateThreadFunc, data);
            }
            voice = voice->next;
        }
        mWorkerThreads->waitFinished();
    }
    // Update
    if (mMasterMode == TIME_MASTER_ASYNC) {
        processInactiveVoices();
    }
}

void DynamicScene::print(ostream &stream) {

    stream << "Audio Distance Attenuation:";
    const char* s = nullptr;
#define PROCESS_VAL(p) case(p): s = #p; break;
    switch(mDistAtten.law()){
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
        int fpb = internalAudioIO.framesPerBuffer();
        SynthVoice *voice = scene->mActiveVoices;
        while (voice) {
            //                if (voice->active()) { // No need to check as only active voices are put in mThreadMap
            if (find(idsToProcess.begin(), idsToProcess.end(), voice->id()) != idsToProcess.end()) { // voice has been assigned to this thread
                int offset = voice->getStartOffsetFrames(fpb);
                if (offset < fpb) {
                    io.frame(offset);
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
                        Vec3d direction = posVoice->pose().vec() - scene->mListenerPose.vec();

                        //Rotate vector according to listener-rotation
                        Quatd srcRot = scene->mListenerPose.quat();
                        listeningDir = srcRot.rotate(direction);
                        posOffsets = posVoice->audioOutOffsets();
                        assert(posOffsets.size() == 0 || posOffsets.size() == posVoice->numOutChannels());
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
                        (*scene->mBusRoutingCallback)(internalAudioIO, scene->mListenerPose);
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
                        scene->mSpatializer->renderBuffer(io, offsetPose, internalAudioIO.outBuffer(i), fpb);
                    }
                    scene->mSpatializerLock.unlock();
                }
            }
            voice = voice->next;
        }
        scene->mAudioBusy--;
        scene->mAudioThreadDone.notify_one();
    }
    std::cout << "Audio thread " << id << " done" << std::endl;
}
