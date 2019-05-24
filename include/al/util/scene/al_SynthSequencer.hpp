
#ifndef AL_SYNTHSEQUENCER_HPP
#define AL_SYNTHSEQUENCER_HPP

/*	Allolib --
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
	Event Voice and Sequencer

	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <map>
#include <vector>
#include <string>
#include <list>
#include <queue>
#include <limits.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <mutex>
#include <algorithm>
#include <functional>
#include <atomic>
#include <thread>
#include <cassert>

#include <typeinfo> // For class name instrospection

#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/io/al_AudioIOData.hpp"
#include "al/core/io/al_File.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/util/scene/al_PolySynth.hpp"

//#include "Gamma/Domain.h"

namespace al
{

class SynthSequencerEvent {
public:
    SynthSequencerEvent () {}

    ~SynthSequencerEvent() {

    }

    typedef enum {
        EVENT_VOICE,
        EVENT_PFIELDS,
        EVENT_TEMPO
    } EventType;

    double startTime {0};
    double duration {-1};
    int offsetCounter {0}; // To offset event within audio buffer

    EventType type {EVENT_VOICE};

    typedef struct {
        std::string name; // instrument name
        std::vector<ParameterField> pFields;
    } ParamFields;

    SynthVoice *voice {nullptr};
    ParamFields fields;
    float tempo;
};

enum SynthEventType {
    TRIGGER_ON,
    TRIGGER_OFF,
    PARAMETER_CHANGE
};

struct SynthEvent {
    std::string synthName;
    double time;
    int id;
    double duration = -1;
    std::vector<ParameterField> pFields;
    SynthEventType type;
};

/**
 * @brief Event Sequencer triggering audio visual "notes"
 *
 * Sequences can be created programatically:
 * @code
    SynthSequencer seq;
    seq.add<SineEnv>( 0  ).set(3.5, 260, 0.3, .011, .2);
    seq.add<SineEnv>( 0  ).set(3.5, 510, 0.3, .011, .2);
    seq.add<SineEnv>( 3.5).set(3.5, 233, 0.3, .011, .2);
    seq.add<SineEnv>( 3.5).set(3.5, 340, 0.3, .011, .2);
    seq.add<SineEnv>( 3.5).set(7.5, 710, 0.3, 1, 2);
 *  @endcode
 *
 * The render() functions need to be places within their relevant contexts like
 * the audio callback (e.g. onSound() ) or the graphics callback (e.g. onDraw())
 *
 * A time master can be selected in the constructor to define where the
 * sequencer runs. TIME_MASTER_AUDIO is more precise in time, but you might want
 * to use TIME_MASTER_GRAPHICS if your "note" produces no audio.
 *
 * Sequences can also be read from text files with the extension
 * ".synthSequence". You need to register the voices used in the sequence
 * with the PolySynth using this->synth().registerSynthClass<MyVoice>("MyVoice")
 * to connect the voice name in the text file to a class name.
 *
 * The following commands are accepted:
 *
 * Event
 * @ absTime duration synthName pFields....
 *
 * e.g. @ 0.981379 0.116669 MyVoice 0 0 1 698.456 0.1 1
 *
 * Turnon
 * + absTime eventId synthName pFields....
 *
 * e.g. + 0.981379 25 MyVoice 0 0 1 698.456 0.1 1
 *
 * Turnoff
 * - absTime eventId
 *
 * eventId looks of the oldest id match adn turns it off
 * e.g. - 1.3 25
 *
 * Tempo
 * t absTime tempoBpm
 *
 * e.g. t 4.5 120
 *
 */

class SynthSequencer {
public:

    SynthSequencer(PolySynth::TimeMasterMode masterMode =  PolySynth::TIME_MASTER_AUDIO)
    {
        mInternalSynth = std::make_unique<PolySynth>(masterMode);
        registerSynth(*mInternalSynth.get());
    }

    SynthSequencer(PolySynth &synth)
    {
        registerSynth(synth);
    }

    /// Insert this function within the audio callback
    void render(AudioIOData &io);

    /// Insert this function within the graphics callback
    void render(Graphics &g);

    /// Set the frame rate at which the graphics run (i.e. how often render(Graphics &g)
    /// will be called
    void setGraphicsFrameRate(float fps) {mFps = fps;} // TODO this should be handled through Gamma Domains

    /**
     * @brief insert an event in the sequencer
     * @param startTime
     * @param duration
     * @return a reference to the voice instance inserted
     *
     * The voice will be inserted into the sequencer immediately, so you may want to
     * add all your notes before starting the sequencer. If you need
     * to insert events on the fly, use addVoice() or use triggerOn()
     * directly on the PolySynth
     *
     * The TSynthVoice template must be a class inherited from SynthVoice.
     */
    template<class TSynthVoice>
    TSynthVoice &add(double startTime, double duration = -1);

    /**
     * Insert configured voice into sequencer.
     */
    template<class TSynthVoice>
    void addVoice(TSynthVoice *voice, double startTime, double duration = -1);

    /**
     * Insert configured voice into sequencer using time offset from current time
     */
    template<class TSynthVoice>
    void addVoiceFromNow(TSynthVoice *voice, double startTime, double duration = -1);

    /**
     * @brief Basic audio callback for quick prototyping
     * @param io
     *
     * Pass this audio callback to an AudioIO object with a pointer to a
     *  SynthSequencer instance to hear the sequence.
     */
    static void audioCB(AudioIOData& io) {
        io.user<SynthSequencer>().render(io);
    }

    /**
     * @brief print current sequence
     */
    void print() {
        std::cout << "POLYSYNTH INFO ................." << std::endl;
        mPolySynth->print();
    }

    bool verbose() {return mVerbose;}
    void verbose(bool verbose) { mVerbose = verbose;}

    void setTempo(float tempo) {mNormalizedTempo = tempo/60.f;}

    bool playSequence(std::string sequenceName, float startTime = 0.0f);

    void stopSequence();

    void registerTimeChangeCallback(std::function<void (float)> func, float minTimeDeltaSec);

    void setTime(float newTime) {  std::cout << "Setting time not implemented" <<std::endl;}

    void setDirectory(std::string directory) {
      assert(directory.size() > 0);
      mDirectory = directory;
    }

    std::string buildFullPath(std::string sequenceName);

    std::list<SynthSequencerEvent> loadSequence(std::string sequenceName, double timeOffset = 0, double timeScale = 1.0);

    std::vector<std::string> getSequenceList();

    double getSequenceDuration(std::string sequenceName);

    PolySynth &synth() {return *mPolySynth;}

    // TODO we should cleanup internal synth if an external one is set
    void registerSynth(PolySynth &synth) {
        mPolySynth = &synth;
        mMasterMode = mPolySynth->mMasterMode;
    }

    void operator<<(PolySynth &synth) { return registerSynth(synth);}

private:
    PolySynth *mPolySynth;
    std::unique_ptr<PolySynth> mInternalSynth;

    std::string mDirectory {"."};
    bool mVerbose{false};

    double mFps {30}; // graphics frames per second

    unsigned int mNextEvent {0};
    std::list<SynthSequencerEvent> mEvents; // List of events sorted by start time.
    std::mutex mEventLock;
    std::mutex mLoadingLock;

    PolySynth::TimeMasterMode mMasterMode {PolySynth::TIME_MASTER_AUDIO};
    double mMasterTime {0.0};
    double mPlaybackStartTime {0.0};

    float mNormalizedTempo {1.0f}; // Linearly normalized inverted around 60 bpm (1.0 = 60bpm, 0.5 = 120 bpm)

    // Time change callback
    std::function<void(float)> mTimeChangeCallback;
    float mTimeChangeMinTimeDelta = 0;
    double mTimeAccumCallbackNs = 0; // Accumulator for tirggering time change callback.

    // CPU processing thread. Used when TIME_MASTER_CPU
    std::shared_ptr<std::thread> mCpuThread;

    void processEvents(double blockStartTime, double fps);

};

//  Implementations -------------

template<class TSynthVoice>
TSynthVoice &SynthSequencer::add(double startTime, double duration) {
    std::list<SynthSequencerEvent>::iterator insertedEvent;
    TSynthVoice *newVoice = mPolySynth->getVoice<TSynthVoice>();
    std::unique_lock<std::mutex> lk(mEventLock);
    // Insert into event list, sorted.
    auto position = mEvents.begin();
    while(position != mEvents.end() && position->startTime < startTime) {
        position++;
    }
    insertedEvent = mEvents.insert(position, SynthSequencerEvent());
    insertedEvent->startTime = startTime;
    insertedEvent->duration = duration;
    insertedEvent->voice = newVoice;
    return *newVoice;
}

template<class TSynthVoice>
void SynthSequencer::addVoice(TSynthVoice *voice, double startTime, double duration) {
    std::list<SynthSequencerEvent>::iterator insertedEvent;
    std::unique_lock<std::mutex> lk(mEventLock);
    // Insert into event list, sorted.
    auto position = mEvents.begin();
    while(position != mEvents.end() && position->startTime < startTime) {
        position++;
    }
    insertedEvent = mEvents.insert(position, SynthSequencerEvent());
    insertedEvent->startTime = startTime;
    insertedEvent->duration = duration;
    insertedEvent->voice = voice;
}

template<class TSynthVoice>
void SynthSequencer::addVoiceFromNow(TSynthVoice *voice, double startTime, double duration)
{
  float triggerOffset = 0.05;
  addVoice(voice, mMasterTime + startTime + triggerOffset, duration);
}

}


#endif  // AL_SYNTHSEQUENCER_HPP
