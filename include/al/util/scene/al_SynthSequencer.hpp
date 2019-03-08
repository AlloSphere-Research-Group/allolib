
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
        for (auto &event: mEvents) {
            if (event.type == SynthSequencerEvent::EVENT_VOICE) {
                // Give back allocated voice to synth
                mPolySynth->insertFreeVoice(event.voice);
            }
        }
        
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

    std::list<SynthSequencerEvent> loadSequence(std::string sequenceName, double timeOffset = 0, double timeScale = 1.0f) {
        std::unique_lock<std::mutex> lk(mLoadingLock);
        std::list<SynthSequencerEvent> events;
        std::string fullName = buildFullPath(sequenceName);
        std::ifstream f(fullName);
        if (!f.is_open()) {
            std::cout << "Could not open:" << fullName << std::endl;
            return events;
        }

        std::string line;
        double tempoFactor = 1.0;
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

                float startTime = std::stof(start) * timeScale * tempoFactor;
                double duration = std::stod(durationText) * timeScale * tempoFactor;

                // const int maxPFields = 64;
                std::vector<ParameterField> pFields;

                // int numFields = 0;
                std::string fieldsString;

                std::getline(ss, fieldsString);
                bool processingString = false;
                std::string stringAccum;
                size_t currentIndex = 0;

                auto isFloat = []( std::string myString ) {
                    std::istringstream iss(myString);
                    float f;
                    iss >> std::noskipws >> f; // noskipws considers leading whitespace invalid
                    // Check the entire string was consumed and if either failbit or badbit is set
                    return iss.eof() && !iss.fail();
                };

                while(currentIndex < fieldsString.size()) {
                    if (fieldsString[currentIndex] == '"') {
                        if (processingString) { // String end
                            pFields.push_back(stringAccum);
                            stringAccum.clear();
                            processingString = false;
                        } else { // String begin
                            processingString = true;
                        }
                    } else if (fieldsString[currentIndex] == ' '
                            || fieldsString[currentIndex] == '\t'
                            || fieldsString[currentIndex] == '\n'
                            || fieldsString[currentIndex] == '\r'
                            ) {
                        if (processingString) {
                            stringAccum += fieldsString[currentIndex];
                        } else if (stringAccum.size() > 0) { // accumulate
                            if (isFloat(stringAccum)) {
                                pFields.push_back(stof(stringAccum));
                            } else {
                                pFields.push_back(stringAccum);
                            }
//                            std::cout << stringAccum << std::endl;
                            stringAccum.clear();
                        }
                    } else { // Accumulate character
                        stringAccum += fieldsString[currentIndex];
                    }
                    currentIndex++;
                }
                if (stringAccum.size() > 0) {
                    if (isFloat(stringAccum)) {
                        pFields.push_back(stof(stringAccum));
                    } else {
                        pFields.push_back(stringAccum);
                    }
                }


                if (false) { // Insert event as EVENT_VOICE. This is not good as it forces preallocating all the events...
                    SynthVoice *newVoice = mPolySynth->getVoice(name);
                    if (newVoice) {
                        if (!newVoice->setParamFields(pFields)) {
                            std::cout << "Error setting pFields for voice of type " << name << ". Fields: ";
                            for (auto &field: pFields) {
                                std::cout << field.type() << " ";
                            }
                            std::cout << std::endl;
                            mPolySynth->insertFreeVoice(newVoice); // Return voice to sequencer.
                        } else {
                            double absoluteTime = timeOffset + startTime;
                            // Insert into event list, sorted.
                            auto position = events.begin();
                            while(position != events.end() && position->startTime < absoluteTime) {
                                position++;
                            }
                            auto insertedEvent = events.insert(position, SynthSequencerEvent());
                            // Add 0.1 padding to ensure all events play.
                            insertedEvent->type = SynthSequencerEvent::EVENT_VOICE;
                            insertedEvent->startTime = absoluteTime;
                            insertedEvent->duration = duration;
                            insertedEvent->voice = newVoice;
    //                        std::cout << "Inserted event " << events.size() << " at time " << absoluteTime << std::endl;
                        }
                    }
                } else {
                    double absoluteTime = timeOffset + startTime;
                    // Insert into event list, sorted.
                    auto position = events.begin();
                    while(position != events.end() && position->startTime < absoluteTime) {
                        position++;
                    }
                    auto insertedEvent = events.insert(position, SynthSequencerEvent());
                    // Add 0.1 padding to ensure all events play.
                    insertedEvent->type = SynthSequencerEvent::EVENT_PFIELDS;
                    insertedEvent->startTime = absoluteTime;
                    insertedEvent->duration = duration;
                    insertedEvent->fields.name = name;
                    insertedEvent->fields.pFields = pFields; // TODO it would be nice not to have to copy here...
                }
               
//                std::cout << "Done reading sequence" << std::endl;
            } else if (command == '+' && ss.get() == ' ') {
                std::string name, idText, start;
                std::getline(ss, start, ' ');
                std::getline(ss, idText, ' ');
                std::getline(ss, name, ' ');

                float startTime = std::stof(start) * timeScale * tempoFactor;
                int id = std::stoi(idText);
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

                SynthVoice *newVoice = mPolySynth->getVoice(name);
                if (newVoice) {
                    newVoice->id(id);
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
                std::getline(ss, idText);
                int id = std::stoi(idText);
                double eventTime = std::stod(time) * timeScale * tempoFactor;
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
            } else if (command == '=' && ss.get() == ' ') {
                std::string time, sequenceName, timeScaleInFile;
                std::getline(ss, time, ' ');
                std::getline(ss, sequenceName, ' ');
                std::getline(ss, timeScaleInFile);
                if (sequenceName.at(0) == '"') {
                    sequenceName = sequenceName.substr(1);
                }
                if (sequenceName.back() == '"') {
                    sequenceName = sequenceName.substr(0, sequenceName.size() - 1);
                }
                lk.unlock();
                auto newEvents = loadSequence(sequenceName, stod(time) + timeOffset, stod(timeScaleInFile) * tempoFactor);
                lk.lock();
                events.insert(events.end(), newEvents.begin(), newEvents.end());
            } else if (command == '>' && ss.get() == ' ') {
                std::string time, sequenceName;
                std::getline(ss, time);
                timeOffset += std::stod(time);
            } else if (command == 't' && ss.get() == ' ') {
                std::string tempo;
                std::getline(ss, tempo);
                tempoFactor = 60.0/std::stod(tempo);
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

        // get list of files ending in ".synthSequence"
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


        std::sort(sequenceList.begin(), sequenceList.end(), [](const auto& lhs, const auto& rhs){
            const auto result = mismatch(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), [](const auto& lhs, const auto& rhs){return tolower(lhs) == tolower(rhs);});

            return result.second != rhs.cend() && (result.first == lhs.cend() || tolower(*result.first) < tolower(*result.second));
        });

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
