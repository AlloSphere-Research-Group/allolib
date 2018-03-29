
#ifndef AL_SYNTHSEQUENCER_HPP
#define AL_SYNTHSEQUENCER_HPP

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
	Event Voice and Sequencer

	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <map>
#include <vector>
#include <list>
#include <limits.h>
#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>
#include <algorithm>

#include <typeinfo> // For class name instrospection
#include <typeindex>


#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/io/al_AudioIOData.hpp"
#include "al/util/al_SingleRWRingBuffer.hpp"

//#include "Gamma/Domain.h"

namespace al
{

int asciiToIndex(int asciiKey, int offset = 0);

int asciiToMIDI(int asciiKey, int offset = 0);

/**
 * @brief The SynthVoice class
 *
 * When inheriting this class you must provide a default construct that takes no arguments
*/
class SynthVoice {
    friend class PolySynth; // PolySynth needs to access private members like "next".
public:

    /// Returns true if voice is currently active
    bool active() { return mActive;}

    /**
     * @brief Override this function to define audio processing.
     * @param io
     *
     * You will need to mark this instance as done by calling the
     *  free() function when envelopes or processing is done. You should
     * call free() from one of the render() functions. You can access the
     * note parameters using the getInstanceParameter(), getParameters()
     * and getOffParameters() functions.
     */
    virtual void onProcess(AudioIOData& io) {}

    /**
     * @brief Override this function to define graphics for this synth
     */
    virtual void onProcess(Graphics &g) {}

    /**
    * @brief  Override this function to determine what needs to be done when note/event starts.
    *
    * When a note starts, internal data within the algorithm usually needs to be reset,
    * e.g. reset envelopes, oscillator phase, etc.
    */
    virtual void onTriggerOn() {}

    /**
    * @brief  determine what needs to be done when note/event ends
    * Define this function to determine what needs to be done when note/event
    * ends. e.g. trigger release in envelopes, etc.
    * */
    virtual void onTriggerOff() {}

    /// This function can be called to programatically trigger  a voice.
    /// It is used for example in PolySynth to trigger a voice.
    void triggerOn(int offsetFrames = 0) {
        mOnOffsetFrames = offsetFrames;
        onTriggerOn();
        mActive = true;
    }

    /// This function can be called to programatically trigger the release
    /// of a voice.
    void triggerOff(int offsetFrames = 0) {
        mOffOffsetFrames = offsetFrames; // TODO implement offset frames for trigger off. Currently ignoring and turning off at start of buffer
        onTriggerOff();
    }
    void id(int idValue) {mId = idValue;}

    int id() {return mId;}

    /**
     * @brief returns the offset frames and sets them to 0.
     * @return offset frames
     *
     * Get the number of frames by which the start of this voice should be offset within a
     * processing block. This value is set to 0 once read as it should only
     * apply on the first rendering pass of a voice.
     */
    int getStartOffsetFrames() {
        int frames = mOnOffsetFrames;
        mOnOffsetFrames = 0;
        return frames;
    }

    int &getEndOffsetFrames() {return mOffOffsetFrames;}

protected:

    ///
    /**
     * @brief Mark this voice as done.
     *
     * This should be set within one of the render()
     * functions when envelope or time is done and no more processing for
     * the note is needed. The voice will be considered ready for retriggering
     * by PolySynth
     */
    void free() {mActive = false; } // Mark this voice as done.

private:
    int mId {-1};
    int mActive {false};
    int mOnOffsetFrames {0};
    int mOffOffsetFrames {0};
    SynthVoice *next {nullptr}; // To support SynthVoices as linked lists
};

class PolySynth {
public:
    typedef enum {
        TIME_MASTER_AUDIO,
        TIME_MASTER_GRAPHICS
    } TimeMasterMode;

    PolySynth(TimeMasterMode masterMode = TIME_MASTER_AUDIO)
        : mMasterMode(masterMode)
    {
    }

    /**
     * @brief trigger Puts voice in active voice lit and calls triggerOn() for it
     * @param voice pointer to the voice to trigger
     * @return a unique id for the voice
     *
     * You can use the id to identify the note for later triggerOff() calls
     */
    int triggerOn(SynthVoice *voice, int offsetFrames = 0, int id = -1);

    /// trigger release of voice with id
    void triggerOff(int id);

    /**
     * @brief Get a reference to a voice.
     * @return
     *
     * Returns a free voice from the internal dynamic allocated pool.
     * You must call triggerVoice to put the voice back in the rendering
     * chain after setting its properties, otherwise it will be lost.
     */
    template<class TSynthVoice>
    TSynthVoice *getVoice();

    /**
     * @brief render all the active voices into the audio buffers
     * @param io AudioIOData containing buffers and audio I/O meta data
     */
    void render(AudioIOData &io);

    /**
     * @brief render graphics for all active voices
     */
    void render(Graphics &g);

    /**
     * Preallocate a number of voices of a particular TSynthVoice to avoid doing realtime
     * allocation.
     */
    template<class TSynthVoice>
    void allocatePolyphony(int number);

    /**
     * @brief Insert an AudioCallback object at the end of the callback queue
     * @param v The AudioCallback object
     * @return this instance
     *
     * The callback provided is appended to the list of post processing callbacks.
     * The post processing callbacks are run in sequence after the audio from all voices
     * has been rendered. This can be useful for global effects, spatialization, etc.
     */
    PolySynth &append(AudioCallback &v);

    /**
     *
     * @brief Insert an AudioCallback object at the head of the callback queue
     * @param v The AudioCallback object
     * @return this instance
     *
     * The callback provided is prepended to the list of post processing callbacks.
     * The post processing callbacks are run in sequence after the audio from all voices
     * has been rendered. This can be useful for global effects, spatialization, etc.
     */
    PolySynth &prepend(AudioCallback &v);

    /**
     *
     * @brief Insert an AudioCallback object before another in the queue
     * @param v The AudioCallback object
     * @param beforeThis The callback that will come after v
     * @return this instance
     *
     * The callback provided is inserted into the list of post processing callbacks.
     *
     * The post processing callbacks are run in sequence after the audio from all voices
     * has been rendered. This can be useful for global effects, spatialization, etc.
     *
     * If beforeThis is not found v is prepended to the callback list
     */
    PolySynth &insertBefore(AudioCallback &v, AudioCallback &beforeThis);

    /**
     *
     * @brief Insert an AudioCallback object after another in the queue
     * @param v The AudioCallback object
     * @param beforeThis The callback that will come before v
     * @return this instance
     *
     * The callback provided is inserted into the list of post processing callbacks.
     *
     * The post processing callbacks are run in sequence after the audio from all voices
     * has been rendered. This can be useful for global effects, spatialization, etc.
     *
     * If afterThis is not found v is prepended to the callback list
     */
    PolySynth &insertAfter(AudioCallback &v, AudioCallback &afterThis);


    /**
     *
     * @brief Remove an AudioCallback object from the queue
     * @param v The AudioCallback object
     * @return this instance
     *
     * The callback provided is removed from the list of post processing callbacks.
     *
     * If v is not found, this call has no effect.
     */
    PolySynth &remove(AudioCallback &v);

    /**
     * @brief prints details of the allocated voices (free, active and queued)
     *
     * Warning: this function is not thread safe and might crash if the audio
     * or graphics is running. Only use for temporary debugging.
     */
    void print();

private:

    // Internal voices are allocated in PolySynth and shared with the outside.
    SynthVoice *mVoicesToInsert {nullptr}; //Voices to be inserted in the realtime context
    SynthVoice *mFreeVoices {nullptr}; // Allocated voices available for reuse
    SynthVoice *mActiveVoices {nullptr}; // Dynamic voices that are currently active. Only modified within the master domain (set by mMasterMode)
    std::mutex mVoiceToInsertLock;
    std::mutex mFreeVoiceLock;
    std::mutex mGraphicsLock;

    SingleRWRingBuffer mVoiceIdsToTurnOff {64 * sizeof(int)};

    TimeMasterMode mMasterMode;

    std::vector<AudioCallback *> mPostProcessing;

    int mIdCounter {0};
};

class SynthSequencerEvent {
public:
    double startTime {0};
    double duration {-1};
    int offsetCounter {0}; // To offset event within audio buffer
    SynthVoice *voice;
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
 */

class SynthSequencer {
public:

    SynthSequencer(PolySynth::TimeMasterMode masterMode =  PolySynth::TIME_MASTER_AUDIO)
        : mPolySynth(masterMode), mMasterMode(masterMode)
    {
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
     * This function is not thread safe, so you must add all your notes before starting the
     * sequencer context (e.g. the audio callback if using TIME_MASTER_AUDIO). If you need
     * to insert events on the fly, use triggerOn() directly on the PolySynth member
     *
     * The TSynthVoice template must be a class inherited from SynthVoice.
     */
    template<class TSynthVoice>
    TSynthVoice &add(double startTime, double duration = -1);

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
     * @brief print debugging information. Is not be thread safe. Use with care.
     */
    void print() {
        std::cout << "POLYSYNTH INFO ................." << std::endl;
        mPolySynth.print();
    }

private:
    PolySynth mPolySynth;

    double mFps {30}; // graphics frames per second

    unsigned int mNextEvent {0};
    std::list<SynthSequencerEvent> mEvents; // List of events sorted by start time.

    PolySynth::TimeMasterMode mMasterMode {PolySynth::TIME_MASTER_AUDIO};
    double mMasterTime {0};

    void processEvents(double blockStartTime, double fps);
};

//  Implementations -------------

template<class TSynthVoice>
TSynthVoice *PolySynth::getVoice() {
    std::unique_lock<std::mutex> lk(mFreeVoiceLock); // Only one getVoice() call at a time
    SynthVoice *freeVoice = mFreeVoices;
    SynthVoice *previousVoice = nullptr;
    while (freeVoice) {
        if (std::type_index(typeid(*freeVoice)) == std::type_index(typeid(TSynthVoice))) {
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
        // TODO report current polyphony for more informed allocation of polyphony
        std::cout << "Allocating voice of type " << typeid (TSynthVoice).name() << "." << std::endl;
        freeVoice = new TSynthVoice;
    }
    return static_cast<TSynthVoice *>(freeVoice);
}

template<class TSynthVoice>
void PolySynth::allocatePolyphony(int number) {
    std::unique_lock<std::mutex> lk(mFreeVoiceLock);
    SynthVoice *lastVoice = mFreeVoices;
    if (lastVoice) {
        while (lastVoice->next) { lastVoice = lastVoice->next; }
    } else {
        lastVoice = mFreeVoices = new TSynthVoice;
        number--;
    }
    for(int i = 0; i < number; i++) {
        lastVoice->next = new TSynthVoice;
        lastVoice = lastVoice->next;
    }
}

// -----

template<class TSynthVoice>
TSynthVoice &SynthSequencer::add(double startTime, double duration) {
    // Insert into event list, sorted.
    auto position = mEvents.begin();
    while(position != mEvents.end() && position->startTime < startTime) {
        position++;
    }
    auto insertedEvent = mEvents.insert(position, SynthSequencerEvent());
    insertedEvent->startTime = startTime;
    insertedEvent->duration = duration;
    TSynthVoice *newVoice = mPolySynth.getVoice<TSynthVoice>();
    insertedEvent->voice = newVoice;
    return *newVoice;
}

}


#endif  // AL_SYNTHSEQUENCER_HPP
