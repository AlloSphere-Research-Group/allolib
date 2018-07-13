
#ifndef AL_DYNAMICSCENE_HPP
#define AL_DYNAMICSCENE_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012-2018. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.

	File description:
	Dynamic Scene Manager

	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <memory>

#include "al/core/spatial/al_Pose.hpp"
#include "al/core/sound/al_AudioScene.hpp"
#include "al/util/scene/al_SynthSequencer.hpp"

#include "al/core/sound/al_StereoPanner.hpp"
#include "al/core/spatial/al_DistAtten.hpp"

using namespace std;

namespace al
{

class PositionedVoice : public SynthVoice {
public:
    Pose &pose() {return mPose;}

    float size() {return mSize;}

    bool useDistanceAttenuation() {return mUseDistAtten;}
    void useDistanceAttenuation(bool atten) {mUseDistAtten = atten;}

protected:
    Pose mPose;
    float mSize {1.0};

    bool mUseDistAtten {true};
};


struct UpdateThreadFuncData {
    SynthVoice *voice;
    double dt;
};

//thread pool from https://stackoverflow.com/questions/23896421/efficiently-waiting-for-all-tasks-in-a-threadpool-to-finish
class ThreadPool
{
public:
    ThreadPool(unsigned int n = std::thread::hardware_concurrency());

    template<class F>
    void enqueue(F &&f, UpdateThreadFuncData &data);
    void waitFinished();
    ~ThreadPool();

    unsigned int size() {return workers.size();}

private:
    std::vector< std::thread > workers;
    std::deque<std::pair<std::function<void(UpdateThreadFuncData)>, UpdateThreadFuncData>> tasks;
    std::mutex queue_mutex;
    std::condition_variable cv_task;
    std::condition_variable cv_finished;
    unsigned int busy;
    bool stop;

    void thread_proc();
};


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

template<class F>
void ThreadPool::enqueue(F&& f, UpdateThreadFuncData &data)
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    tasks.emplace_back(std::pair<std::function<void(UpdateThreadFuncData)>, UpdateThreadFuncData>(std::forward<F>(f), data));
    cv_task.notify_one();
}

// waits until the queue is empty.

void ThreadPool::waitFinished()
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    cv_finished.wait(lock, [this](){ return tasks.empty() && (busy == 0); });
}


class DynamicScene : public PolySynth
{
public:
    DynamicScene (int threadPoolSize = 0) {
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
    }

    virtual ~DynamicScene () {
        mSynthRunning = false;
        mThreadTrigger.notify_all();
        if (mWorkerThreads) {
            mWorkerThreads->waitFinished();
        }
        for (auto &thr : mAudioThreads) {
            thr.join();
        }
    }

    /// Set a Spatializar to use. If not called, the default is stereo panning
    /// over two speakers.
    template<class TSpatializer>
    shared_ptr<TSpatializer> setSpatializer(SpeakerLayout &sl) {
        mSpatializer = make_shared<TSpatializer>(sl);
        mSpatializer->compile();
        return static_pointer_cast<TSpatializer>(mSpatializer);
    }

    /// Prepares internals for run. This function must be called before any
    /// render() calls
    void prepare(AudioIOData &io) {
        int mVoiceMaxOutputChannels = 2;
        int mVoiceMaxInputChannels = 0;
        int mVoiceMaxBusChannels = 0;
        // This will probably need to be moved to SynthVoice? Otherwise how can we know here how to set things up... all voices may have different needs...
        internalAudioIO.framesPerBuffer(io.framesPerBuffer());
        internalAudioIO.channelsIn(mVoiceMaxInputChannels);
        internalAudioIO.channelsOut(mVoiceMaxOutputChannels);
        internalAudioIO.channelsBus(mVoiceMaxBusChannels);
        mThreadedAudioData.resize(mAudioThreads.size());
        for (auto &threadio: mThreadedAudioData) {
            threadio.framesPerBuffer(io.framesPerBuffer());
            threadio.channelsIn(mVoiceMaxInputChannels);
            threadio.channelsOut(mVoiceMaxOutputChannels);
            threadio.channelsBus(mVoiceMaxBusChannels);
        }
//        mSpatializer->prepare(io);
    }

    virtual void cleanup() {

    }

    /// The listener pose is used to determine both the graphic view
    /// and the audio spatialization
    Pose &listenerPose() {return mListenerPose;}
    void listenerPose(Pose &pose) {mListenerPose = pose;}

    virtual void render(Graphics &g) final {

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

    virtual void render(AudioIOData &io) final {
        assert(mSpatializer && "ERROR: call setSpatializer before starting audio");
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
                        io.frame(offset);
                        int endOffsetFrames = voice->getEndOffsetFrames(fpb);
                        if (endOffsetFrames > 0 && endOffsetFrames <= fpb) {
                            voice->triggerOff(endOffsetFrames);
                        }
                        internalAudioIO.zeroOut();
                        internalAudioIO.frame(0);
                        voice->onProcess(internalAudioIO);
                        Pose listeningPose;
                        if (dynamic_cast<PositionedVoice *>(voice)) {
                            PositionedVoice *posVoice = static_cast<PositionedVoice *>(voice);
                            listeningPose = posVoice->pose() * mListenerPose;
                            if (posVoice->useDistanceAttenuation()) {
                                float distance = listeningPose.vec().mag();
                                float atten = mDistAtten.attenuation(distance);
                                internalAudioIO.frame(0);
                                float *buf = internalAudioIO.outBuffer(0);

                                while (internalAudioIO()) {
                                    *buf = *buf * atten;
                                    buf++;
                                }
                            }
                        } else {
                            listeningPose = mListenerPose;
                        }
                        mSpatializer->renderBuffer(io, listeningPose, internalAudioIO.outBuffer(0), fpb);
                    }
                }
                voice = voice->next;
            }
        } else { // Process Audio Threaded
            mAudioBusy = 0;
            for (auto& tmap: mThreadMap) {
                tmap.second.resize(0);
            }
            int counter = 0;
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

    virtual void update(double dt = 0) final {
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

    void setUpdateThreaded(bool threaded) { mThreadedUpdate = threaded; }
    void setAudioThreaded(bool threaded) { mThreadedAudio = threaded; }

    DistAtten<> &distanceAttenuation() {return mDistAtten;}

    void print(ostream &stream = std::cout) {

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
    }


private:

    // A speaker layout and spatializer
    std::shared_ptr<Spatializer> mSpatializer;

    AudioIOData internalAudioIO;

    Pose mListenerPose;
    DistAtten<> mDistAtten;

    // For threaded simulation
    std::unique_ptr<ThreadPool> mWorkerThreads; // Update worker threads
    bool mThreadedUpdate {true};

    // For threaded audio
    bool mThreadedAudio {true};
    std::vector<std::thread> mAudioThreads;
    std::vector<AudioIOData> mThreadedAudioData;
    std::map<int, vector<int>> mThreadMap; // Defines which threads run which voices. Key is thread id, value is voice ids.
    std::condition_variable mThreadTrigger;
    std::condition_variable mAudioThreadDone;
    std::mutex mSpatializerLock;
    AudioIOData *externalAudioIO; // This is captured by the audio callback and passed to the audio threads. Protected by mSpatializerLock
    std::mutex mThreadTriggerLock;
    bool mSynthRunning {true};
    unsigned int mAudioBusy = 0;


    static void updateThreadFunc(UpdateThreadFuncData data) {
//        UpdateThreadFuncData *data = (UpdateThreadFuncData *) ud;
//        std::cout << "eq " << data.voice << "  " << data.dt << std::endl;
        SynthVoice *voice = data.voice;
        double &dt = data.dt;
        voice->update(dt);
    }

    static void audioThreadFunc(DynamicScene *scene, int id) {
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
                        internalAudioIO.frame(0);
                        voice->onProcess(internalAudioIO);
                        if (dynamic_cast<PositionedVoice *>(voice)) {
                            PositionedVoice *posVoice = static_cast<PositionedVoice *>(voice);
                            Pose listeningPose = posVoice->pose() * scene->mListenerPose;
                            if (posVoice->useDistanceAttenuation()) {
                                float distance = listeningPose.vec().mag();
                                float atten = scene->mDistAtten.attenuation(distance);
                                internalAudioIO.frame(0);
                                float *buf = internalAudioIO.outBuffer(0);

                                while (internalAudioIO()) {
                                    *buf = *buf * atten;
                                    buf++;
                                }
                            }
                            scene->mSpatializerLock.lock();
                            io.frame(offset);
                            scene->mSpatializer->renderBuffer(io, listeningPose, internalAudioIO.outBuffer(0), fpb);
                            scene->mSpatializerLock.unlock();
                        } else {
                            // FIXME what should we do here if voice not a PositionedVoice?
                        }
                    }
                }
                voice = voice->next;
            }
            scene->mAudioBusy--;
            scene->mAudioThreadDone.notify_one();
        }
        std::cout << "Audio thread " << id << " done" << std::endl;
    }


};




}


#endif  // AL_DYNAMICSCENE_HPP
