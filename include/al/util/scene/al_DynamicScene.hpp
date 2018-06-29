
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


class DynamicScene : public PolySynth
{
public:
    DynamicScene () {
        SpeakerLayout sl = StereoSpeakerLayout(); // Stereo by default
        setSpatializer<StereoPanner>(sl);
    }

    virtual ~DynamicScene () {}

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
        internalAudioIO.framesPerBuffer(io.framesPerBuffer());
        internalAudioIO.channelsIn(io.channelsIn());
        internalAudioIO.channelsOut(io.channelsOut());
        internalAudioIO.channelsBus(io.channelsBus());
//        mSpatializer->prepare(io);
    }

    virtual void cleanup() {

    }

    Pose &listenerPose() {return mListenerPose;}
    void listenerPose(Pose &pose) {mListenerPose = pose;}

    virtual void render(Graphics &g) override {

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

    virtual void render(AudioIOData &io) override {
        assert(mSpatializer && "ERROR: call setSpatializer before starting audio");
        mSpatializer->prepare(io);
        if (mMasterMode == TIME_MASTER_AUDIO) {
            processVoices();
            // Turn off voices
            processVoiceTurnOff();
        }

        // Render active voices
        auto voice = mActiveVoices;
        int fpb = internalAudioIO.framesPerBuffer();
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
                    if (dynamic_cast<PositionedVoice *>(voice)) {
                        PositionedVoice *posVoice = static_cast<PositionedVoice *>(voice);
                        Pose listeningPose = posVoice->pose() * mListenerPose;
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
                        mSpatializer->renderBuffer(io, listeningPose, internalAudioIO.outBuffer(0), fpb);
                    }
                }
            }
            voice = voice->next;
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

    virtual void update(double dt = 0) override {
        if (mMasterMode == TIME_MASTER_ASYNC) {
            processVoices();
            // Turn off voices
            processVoiceTurnOff();
        }

        // Update active voices
        auto voice = mActiveVoices;
        while (voice) {
            if (voice->active()) {
                voice->update(dt);
            }
            voice = voice->next;
        }
        if (mMasterMode == TIME_MASTER_ASYNC) {
            processInactiveVoices();
        }
    }

    DistAtten<> &distanceAttenuation() {return mDistAtten;}

    void print(ostream &stream = std::cout) {

        stream << "Distance Attenuation:";
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
//    SpeakerLayout sl {StereoSpeakerLayout()};
    std::shared_ptr<Spatializer> mSpatializer;

    AudioIOData internalAudioIO;

    Pose mListenerPose;
    DistAtten<> mDistAtten;
};




}


#endif  // AL_DYNAMICSCENE_HPP
