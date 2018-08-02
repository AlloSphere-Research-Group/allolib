
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

#include <typeinfo> // For class name instrospection
#include <typeindex>


#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/io/al_AudioIOData.hpp"
#include "al/core/io/al_File.hpp"
#include "al/util/al_SingleRWRingBuffer.hpp"
#include "al/util/ui/al_Parameter.hpp"

//#include "Gamma/Domain.h"

namespace al
{

std::string demangle(const char* name); // Utility function. Should not be used by general users

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

    SynthVoice() {}

    virtual ~SynthVoice() {}

    /// Returns true if voice is currently active
    bool active() { return mActive;}

    /**
     * @brief Set parameter values
     * @param pFields array containing the values
     * @param numFields number of fields to process
     * @return true if able to set the fields
     *
     */
    virtual bool setParamFields(float *pFields, int numFields = -1) {
        if (numFields > (int) mParametersToFields.size()) {
            return false;
        }
        for (int i = 0; i < numFields; i++) {
            *mParametersToFields[i] = *pFields++;
        }
        return true;
    }

    /**
     * @brief Get this instance's parameter fields
     * @param pFields a pre-allocated array where the parameter fields will be written.
     * @param maxParams the maximum number of parameters to process (i.e. the allocated size of pFields)
     * @return the number of parameters written
     * 
     * The default behavior is to copy the values from the internal parameters that have been
     * registered using registerParameterAsField or the << operator. Override
     * this function in your voice if you need a different behavior.
     */
    virtual int getParamFields(float *pFields, int maxParams = -1) {
        if (maxParams == -1) {
            maxParams = mParametersToFields.size();
        }
        int count = 0;
        for (auto param: mParametersToFields) {
            if (count == maxParams) {
                break;
            }
            *pFields++ = *param; 
            count++;
        }
        return count;
        }

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
     * @brief Override this function to update internal state, e.g. from an asynchronous simulator
     * 
     * dt is the delta time elapsed since last update 
     */
    virtual void update(double dt = 0) {}

    /**
     * @brief Override this function to initialize internal data.
     * 
     * This function should be called only once upon voice creation.
     */
    virtual void init() {}

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
     * @param framesPerBuffer number of frames per buffer
     * @return offset frames
     *
     * Get the number of frames by which the start of this voice should be offset within a
     * processing block. This value is decremented by framesPerBuffer once read.
     */
    int getStartOffsetFrames(int framesPerBuffer) {
        int frames = mOnOffsetFrames;
        mOnOffsetFrames -= framesPerBuffer;
        if (mOnOffsetFrames < 0) {mOnOffsetFrames = 0.0;}
        return frames;
    }

    int getEndOffsetFrames(int framesPerBuffer) {
        int frames = mOffOffsetFrames;
        mOffOffsetFrames -= framesPerBuffer;
        if (mOffOffsetFrames < 0) {mOffOffsetFrames = 0.0;}
        return frames;
    }

    void userData(void *ud) {mUserData = ud;}

    void *userData() {return mUserData;}

    unsigned int numOutChannels() { return mNumOutChannels; }

    SynthVoice& registerParameterAsField(Parameter &param) { mParametersToFields.push_back(&param); return *this;}

    SynthVoice& operator<<(Parameter &param) {return registerParameterAsField(param);}

    SynthVoice *next {nullptr}; // To support SynthVoices as linked lists

protected:

    ///
    /**
     * @brief Mark this voice as done.
     *
     * This should be set within one of the render()
     * functions when envelope or time is done and no more processing for
     * the note is needed. The voice will be considered ready for retriggering
     * by PolySynth.
     */
    void free() {mActive = false; } // Mark this voice as done.
    /**
     * @brief Set the number of outputs this SynthVoice generates
     * @param numOutputs
     *
     * If you are using this voice within PolySynth, make sure this number is
     * less or equal than the number of output channels opened for the audio
     * device. If using in DynamicScene, make sure
     */
    void setNumOutChannels(unsigned int numOutputs) {mNumOutChannels = numOutputs;}

private:
    int mId {-1};
    int mActive {false};
    int mOnOffsetFrames {0};
    int mOffOffsetFrames {0};
    unsigned int mNumOutChannels {1}; // Set this
    void *mUserData;
    std::vector<Parameter *> mParametersToFields;
};

class PolySynth {
public:
    typedef enum {
        TIME_MASTER_AUDIO,
        TIME_MASTER_GRAPHICS,
        TIME_MASTER_ASYNC
    } TimeMasterMode;

    friend class SynthSequencer;

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
    int triggerOn(SynthVoice *voice, int offsetFrames = 0, int id = -1, void *userData = nullptr);

    /// trigger release of voice with id
    void triggerOff(int id);

    /**
     * @brief Call TriggerOff for all active notes
     */
    void allNotesOff();

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
     * @brief Get a reference to a free voice by voice type name
     * @return
     *
     * Returns a free voice from the internal dynamic allocated pool.
     * If voice is not available. It will be allocated. Can return
     * nullptr if the class name and creator have not been registered
     * with registerSynthClass()
     */
    SynthVoice *getVoice(std::string name);

    /**
     * @brief render all the active voices into the audio buffers
     * @param io AudioIOData containing buffers and audio I/O meta data
     */
    virtual void render(AudioIOData &io);

    /**
     * @brief render graphics for all active voices
     */
    virtual void render(Graphics &g);

    /**
     * @brief update internal state for all voices.
     */
    virtual void update(double dt = 0);

    /**
     * Preallocate a number of voices of a particular TSynthVoice to avoid doing realtime
     * allocation.
     */
    template<class TSynthVoice>
    void allocatePolyphony(int number);

    /**
     * Preallocate a number of voices of a voice to avoid doing realtime
     * allocation. The name must be registered using registerSynthClass()
     */
    void allocatePolyphony(std::string name, int number);

    /**
     * @brief Use this function to insert a voice allocated externally into the free voice pool
     * @param voice the new voice to be added to the free voice pool
     */
    void insertFreeVoice(SynthVoice *voice);


    /**
     * @brief Set default user data to set to voices before the are returned
     * by getVoice()
     * @param userData
     */
    void setDefaultUserData(void *userData) { mDefaultUserData = userData;}

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

    /**
     * @brief Set audio output gain
     * @param gainValue
     */
    void gain(float gainValue) {mAudioGain = gainValue;}

    /**
     * @brief get current audio gain
     * @return
     */
    float gain() {return mAudioGain;}

    /**
     * @brief register a callback to be notified of a trigger on event
     */
    void registerTriggerOnCallback(std::function<void(SynthVoice *voice, int offsetFrames, int id, void *userData)> cb, void *userData) {
        TriggerOnCallback cbNode(cb, userData);
        mTriggerOnCallbacks.push_back(cbNode);
    }

    /**
     * @brief register a callback to be notified of a trigger off event
     */
    void registerTriggerOffCallback(std::function<void(int id, void *userData)> cb, void *userData) {
        TriggerOffCallback cbNode(cb, userData);
        mTriggerOffCallbacks.push_back(cbNode);
    }

    /**
     * Register a SynthVoice class to allow instantiating it by name
     *
     * This is needed for remote instantiation and for text sequence playback.
     */
    template<class TSynthVoice>
    void registerSynthClass(std::string name, bool allowAutoAllocation = true) {
        if (mCreators.find(name) != mCreators.end()) {
            std::cout << "Warning: Overriding registration of SynthVoice: " << name << std::endl;
        }
        if (std::find(mNoAllocationList.begin(), mNoAllocationList.end(), name) == mNoAllocationList.end()) {
            mNoAllocationList.push_back(name);
        }
        mCreators[name] = []() {
            TSynthVoice *voice = new TSynthVoice;
            voice->init();
            return voice;
        };
    }

    SynthVoice *allocateVoice(std::string name) {
        if (mCreators.find(name) != mCreators.end()) {
            std::cout << "Allocating (from name) voice of type " << name << "." << std::endl;
            return mCreators[name]();
        } else {
            std::cout << "Can't allocate voice of type " << name << ". Voice not registered and no polyphony." << std::endl;
        }
        return nullptr;
    }

    // Testing function. Do not use...
    SynthVoice *getActiveVoices() {
        return mActiveVoices;
    }

protected:
    inline void processVoices() {
        if (mVoiceToInsertLock.try_lock()) {
            if (mVoicesToInsert) {
                // If lock acquired insert queued voices
                if (mActiveVoices) {
                    auto voice = mVoicesToInsert;
                    while (voice->next) { // Find last voice to insert
                        voice = voice->next;
                    }
                    voice->next = mActiveVoices; // Connect last inserted to previously active
                    mActiveVoices = mVoicesToInsert; // Put new voices in head
                } else {
                    mActiveVoices = mVoicesToInsert;
                }
                mVoicesToInsert = nullptr;
            }
            mVoiceToInsertLock.unlock();
        }
        if (mAllNotesOff) {
            if (mFreeVoiceLock.try_lock()) {
                mAllNotesOff = false;
                if (mActiveVoices) {
                    auto voice = mActiveVoices->next;
                    SynthVoice *previousVoice = mActiveVoices;
                    previousVoice->id(-1);
                    while(voice) {
                        voice->id(-1);
                        previousVoice = voice;
                        voice = voice->next;
                    }
                    previousVoice->next = mFreeVoices; // Connect last active voice to first free voice
                    mFreeVoices = mActiveVoices; // Move all voices to free voices
                    mActiveVoices = nullptr; // No active voices left
                }
                mFreeVoiceLock.unlock();
            }
        }
    }

    inline void processVoiceTurnOff() {
        int voicesToTurnOff[16];
        int numVoicesToTurnOff;
        while ( (numVoicesToTurnOff = mVoiceIdsToTurnOff.read((char *) voicesToTurnOff, 16 * sizeof (int))) ) {
            for (int i = 0; i < numVoicesToTurnOff/int(sizeof (int)); i++) {
                auto voice = mActiveVoices;
                while (voice) {
                    if (voice->id() == voicesToTurnOff[i]) {
    //                    std::cout << "Voice off "<<  voice->id() << std::endl;
                        voice->triggerOff(); // TODO use offset for turn off
                    }
                    voice = voice->next;
                }
            }
        }
    }

    inline void processInactiveVoices() {
        // Move inactive voices to free queue
        if (mFreeVoiceLock.try_lock()) { // Attempt to remove inactive voices without waiting.
            auto voice = mActiveVoices;
            SynthVoice *previousVoice = nullptr;
            while(voice) {
                if (!voice->active()) {
                    voice->id(-1); // Reset voice id
                    if (previousVoice) {
                        previousVoice->next = voice->next; // Remove from active list
                        voice->next = mFreeVoices;
                        mFreeVoices = voice; // Insert as head in free voices
                        voice = previousVoice; // prepare next iteration
                    } else { // Inactive is head of the list
                        mActiveVoices = voice->next; // Remove voice from list
                        voice->next = mFreeVoices;
                        mFreeVoices = voice; // Insert as head in free voices
                        voice = mActiveVoices; // prepare next iteration
                    }
                }
                previousVoice = voice;
                if (voice) {
                    voice = voice->next;
                }
            }
            mFreeVoiceLock.unlock();
        }
    }

    inline void processGain(AudioIOData &io) {
        io.frame(0);
        if (mAudioGain != 1.0) {
            for (unsigned int i = 0; i < io.channelsOut(); i++) {
                float *buffer = io.outBuffer(i);
                unsigned int samps = io.framesPerBuffer();
                while (samps--) {
                    *buffer++ *= mAudioGain;
                }
            }
        }
    }

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

    typedef std::pair<std::function<void(SynthVoice *voice, int offsetFrames, int id, void *)>, void *> TriggerOnCallback;
    std::vector<TriggerOnCallback> mTriggerOnCallbacks;

    typedef std::pair<std::function<void(int id, void *)>, void *> TriggerOffCallback;
    std::vector<TriggerOffCallback> mTriggerOffCallbacks;

    float mAudioGain {1.0};

    int mIdCounter {0};

    bool mAllNotesOff {false}; // Flag used to notify processing to turn off all voices

    typedef std::function<SynthVoice *()> VoiceCreatorFunc;
    typedef std::map<std::string, VoiceCreatorFunc> Creators;

    void *mDefaultUserData {nullptr};

    Creators mCreators;
    std::vector<std::string> mNoAllocationList; // Disallow auto allocation for class name. Set in allocateVoice()
};

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

    typedef struct {
        int numFields;
        float *pFields = nullptr;
    } ParamFields;

    union {
        SynthVoice *voice;
        ParamFields fields;
        float tempo;
    };
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
    std::vector<float> pFields;
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
     * @brief print current sequence
     */
    void print() {
        std::cout << "POLYSYNTH INFO ................." << std::endl;
        mPolySynth->print();
    }

    void setTempo(float tempo) {mNormalizedTempo = tempo/60.f;}

    bool playSequence(std::string sequenceName) {
//        synth().allNotesOff();
        // Add an offset of 0.1 to make sure the allNotesOff message gets processed before the sequence
        std::list<SynthSequencerEvent> events = loadSequence(sequenceName, mMasterTime + 0.1);
        std::unique_lock<std::mutex> lk(mEventLock);
        mEvents = events;
        mNextEvent = 0;
        return true;
    }

    void stopSequence() {
        std::unique_lock<std::mutex> lk(mEventLock);
        mEvents.clear();
        mNextEvent = 0;
    }

    std::string buildFullPath(std::string sequenceName)
    {
        std::string fullName = mDirectory;

        if (fullName.back() != '/') {
            fullName += "/";
        }
        if (sequenceName.size() < 14 || sequenceName.substr(sequenceName.size() - 14) != ".synthSequence") {
            sequenceName += ".synthSequence";
        }
        fullName += sequenceName;
        return fullName;
    }

    std::list<SynthSequencerEvent> loadSequence(std::string sequenceName, double timeOffset = 0) {
        std::unique_lock<std::mutex> lk(mLoadingLock);
        std::list<SynthSequencerEvent> events;
        std::string fullName = buildFullPath(sequenceName);
        std::ifstream f(fullName);
        if (!f.is_open()) {
            std::cout << "Could not open:" << fullName << std::endl;
            return events;
        }

        std::string line;
        while (getline(f, line)) {
            if (line.substr(0, 2) == "::") {
                break;
            }
            std::stringstream ss(line);
            int command = ss.get();
            if (command == '@' && ss.get() == ' ') {
                std::string name, start, durationText;
                std::getline(ss, start, ' ');
                std::getline(ss, durationText, ' ');
                std::getline(ss, name, ' ');

                float startTime = std::stof(start);
                double duration = std::stod(durationText);

                SynthVoice *newVoice = mPolySynth->getVoice(name);
                if (newVoice) {
                    const int maxPFields = 64;
                    float pFields[maxPFields];

                    int numFields = 0;
                    std::string field;
                    std::getline(ss, field, ' ');
                    while (field != "" && numFields < maxPFields) {
                        pFields[numFields] = std::stof(field);
                        numFields++;
                        std::getline(ss, field, ' ');
                    }
//                    std::cout << "Pfields: ";
//                    for (int i = 0; i < numFields; i++) {
//                        std::cout << pFields[i] << " ";
//                    }
//                    std::cout << std::endl;


                    if (!newVoice->setParamFields(pFields, numFields)) {
                        std::cout << "Error setting pFields for voice of type " << name << ". Fields: ";
                        for (int i = 0; i < numFields; i++) {
                            std::cout << pFields[i] << " ";
                        }
                        std::cout << std::endl;
                    } else {
                        std::list<SynthSequencerEvent>::iterator insertedEvent;
                        double absoluteTime = timeOffset + startTime;
                        // Insert into event list, sorted.
                        auto position = events.begin();
                        while(position != events.end() && position->startTime < absoluteTime) {
                            position++;
                        }
                        insertedEvent = events.insert(position, SynthSequencerEvent());
                        // Add 0.1 padding to ensure all events play.
                        insertedEvent->startTime = absoluteTime;
                        insertedEvent->duration = duration;
                        insertedEvent->voice = newVoice;
//                        std::cout << "Inserted event " << events.size() << " at time " << absoluteTime << std::endl;
                    }
                }
//                std::cout << "Done reading sequence" << std::endl;
            } else if (command == '+' && ss.get() == ' ') {
                std::string name, idText, start;
                std::getline(ss, start, ' ');
                std::getline(ss, idText, ' ');
                std::getline(ss, name, ' ');

                float startTime = std::stof(start);
                int id = std::stoi(idText);

                SynthVoice *newVoice = mPolySynth->getVoice(name);
                if (newVoice) {
                    newVoice->id(id);
                    const int maxPFields = 64;
                    float pFields[maxPFields];

                    int numFields = 0;
                    std::string field;
                    std::getline(ss, field, ' ');
                    while (field != "" && numFields < maxPFields) {
                        pFields[numFields] = std::stof(field);
                        numFields++;
                        std::getline(ss, field, ' ');
                    }
//                    std::cout << "Pfields: ";
//                    for (int i = 0; i < numFields; i++) {
//                        std::cout << pFields[i] << " ";
//                    }
//                    std::cout << std::endl;
                    if (!newVoice->setParamFields(pFields, numFields)) {
                        std::cout << "Error setting pFields for voice of type " << name << ". Fields: ";
                        for (int i = 0; i < numFields; i++) {
                            std::cout << pFields[i] << " ";
                        }
                        std::cout << std::endl;
                    } else {
                        std::list<SynthSequencerEvent>::iterator insertedEvent;
                        double absoluteTime = timeOffset + startTime;
                        {
//                            // Insert into event list, sorted.
                            auto position = events.begin();
                            while(position != events.end() && position->startTime < absoluteTime) {
                                position++;
                            }
                            insertedEvent = events.insert(position, SynthSequencerEvent());
                        }
                        // Add 0.1 padding to ensure all events play.
                        insertedEvent->startTime = absoluteTime;
                        insertedEvent->duration = -1; // Turn on events have undetermined duration until a turn off is found later
                        insertedEvent->voice = newVoice;
//                        std::cout << "Inserted event " << id << " at time " << startTime << std::endl;
                    }
                } else {
                    std::cout << "Warning: Unable to get free voice from PolySynth." << std::endl;
                }
            } else if (command == '-' && ss.get() == ' ') {
                std::string time, idText;
                std::getline(ss, time, ' ');
                std::getline(ss, idText, ' ');
                int id = std::stoi(idText);
                double eventTime = std::stod(time);
                for (SynthSequencerEvent &event: events) {
                    if (event.voice->id() == id && event.duration < 0) {
                        double duration = eventTime - event.startTime + timeOffset;
                        if (duration < 0) {
                            duration = 0;
                        }
                        event.duration = duration;
//                        std::cout << "Set event duration " << id << " to " << duration << std::endl;
                        break;
                    }
                }
            }  else if (command == 't' && ss.get() == ' ') {
                std::string time, idText;
                std::getline(ss, time, ' ');
                std::getline(ss, idText, ' ');
                int id = std::stoi(idText);
                double eventTime = std::stod(time);
                for (SynthSequencerEvent &event: events) {
                    if (event.voice->id() == id && event.duration < 0) {
                        double duration = eventTime - event.startTime + timeOffset;
                        if (duration < 0) {
                            duration = 0;
                        }
                        event.duration = duration;
//                        std::cout << "Set event duration " << id << " to " << duration << std::endl;
                        break;
                    }
                }
            } else {
                if (command > 0) {
                    std::cout << "Line ignored. Command: " << command << std::endl;
                }
            }
        }
        f.close();
        if (f.bad()) {
            std::cout << "Error reading:" << fullName << std::endl;
        }
        return events;
    }


    std::vector<std::string> getSequenceList() {
        std::vector<std::string> sequenceList;
        std::string path = mDirectory;
        if (!File::isDirectory(path)) {
            Dir::make(path);
        }

        // get list of files ending in ".sequence"
        FileList sequence_files = filterInDir(path, [](const FilePath& f){
            if (al::checkExtension(f, ".synthSequence")) return true;
            else return false;
        });

        // store found preset files
        for (int i = 0; i < sequence_files.count(); i += 1) {
            const FilePath& path = sequence_files[i];
            const std::string& name = path.file();
            // exclude extension when adding to sequence list
            sequenceList.push_back(name.substr(0, name.size()-14));
        }
        return sequenceList;
    }

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

    double mFps {30}; // graphics frames per second

    unsigned int mNextEvent {0};
    std::list<SynthSequencerEvent> mEvents; // List of events sorted by start time.
    std::mutex mEventLock;
    std::mutex mLoadingLock;

    PolySynth::TimeMasterMode mMasterMode {PolySynth::TIME_MASTER_AUDIO};
    double mMasterTime {0.0};

    float mNormalizedTempo {1.0f}; // Linearly normalized inverted around 60 bpm (1.0 = 60bpm, 0.5 = 120 bpm)

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
        // TODO check if allocation allowed
        std::cout << "Allocating voice of type " << typeid (TSynthVoice).name() << "." << std::endl;
        freeVoice = new TSynthVoice;
        freeVoice->init();
    }
    freeVoice->userData(mDefaultUserData);
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

}


#endif  // AL_SYNTHSEQUENCER_HPP
