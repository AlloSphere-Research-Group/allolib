//

#include "al/util/ui/al_SynthSequencer.hpp"

using namespace al;

int al::asciiToIndex(int asciiKey, int offset) {
    switch(asciiKey){
    case '1': return offset + 0;
    case '2': return offset + 1;
    case '3': return offset + 2;
    case '4': return offset + 3;
    case '5': return offset + 4;
    case '6': return offset + 5;
    case '7': return offset + 6;
    case '8': return offset + 7;
    case '9': return offset + 8;
    case '0': return offset + 9;

    case 'q': return offset + 10;
    case 'w': return offset + 11;
    case 'e': return offset + 12;
    case 'r': return offset + 13;
    case 't': return offset + 14;
    case 'y': return offset + 15;
    case 'u': return offset + 16;
    case 'i': return offset + 17;
    case 'o': return offset + 18;
    case 'p': return offset + 19;
    case 'a': return offset + 20;

    case 's': return offset + 21;
    case 'd': return offset + 22;
    case 'f': return offset + 23;
    case 'g': return offset + 24;
    case 'h': return offset + 25;
    case 'j': return offset + 26;
    case 'k': return offset + 27;
    case 'l': return offset + 28;
    case ';': return offset + 29;

    case 'z': return offset + 30;
    case 'x': return offset + 31;
    case 'c': return offset + 32;
    case 'v': return offset + 33;
    case 'b': return offset + 34;
    case 'n': return offset + 35;
    case 'm': return offset + 36;
    case ',': return offset + 37;
    case '.': return offset + 38;
    case '/': return offset + 39;
    }
    return 0;
}

int al::asciiToMIDI(int asciiKey, int offset) {
    switch(asciiKey){
    //	case '1': return offset + 0;
    case '2': return offset + 73;
    case '3': return offset + 75;
        //	case '4': return offset + 3;
    case '5': return offset + 78;
    case '6': return offset + 80;
    case '7': return offset + 82;
        //	case '8': return offset + 7;
    case '9': return offset + 85;
    case '0': return offset + 87;

    case 'q': return offset + 72;
    case 'w': return offset + 74;
    case 'e': return offset + 76;
    case 'r': return offset + 77;
    case 't': return offset + 79;
    case 'y': return offset + 81;
    case 'u': return offset + 83;
    case 'i': return offset + 84;
    case 'o': return offset + 86;
    case 'p': return offset + 88;

        //	case 'a': return offset + 20;
    case 's': return offset + 61;
    case 'd': return offset + 63;
        //	case 'f': return offset + 23;
    case 'g': return offset + 66;
    case 'h': return offset + 68;
    case 'j': return offset + 70;
        //	case 'k': return offset + 27;
    case 'l': return offset + 73;
    case ';': return offset + 75;

    case 'z': return offset + 60;
    case 'x': return offset + 62;
    case 'c': return offset + 64;
    case 'v': return offset + 65;
    case 'b': return offset + 67;
    case 'n': return offset + 69;
    case 'm': return offset + 71;
    case ',': return offset + 72;
    case '.': return offset + 74;
    case '/': return offset + 76;
    }
    return 0;
}

//  ----

int PolySynth::triggerOn(SynthVoice *voice, int offsetFrames, int id) {
    assert(voice);
    int thisId = id;
    if (thisId == -1) {
        thisId = mIdCounter++;
    }
    voice->id(thisId);
    voice->triggerOn(offsetFrames);
    std::unique_lock<std::mutex> lk(mVoiceToInsertLock);
    voice->next = mVoicesToInsert;
    mVoicesToInsert = voice;
    return thisId;
}

void PolySynth::triggerOff(int id) {
    mVoiceIdsToTurnOff.write((const char*) &id, sizeof (int));
}

void PolySynth::render(AudioIOData &io) {
    // TODO implement turn off
    if (mMasterMode == TIME_MASTER_AUDIO) {
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
    }
    int voicesToTurnOff[8];
    int numVoicesToTurnOff;
    while (numVoicesToTurnOff = mVoiceIdsToTurnOff.read((char *) voicesToTurnOff, sizeof (int))) {
        for (int i = 0; i < numVoicesToTurnOff; i++) {
            auto voice = mActiveVoices;
            while (voice) {
                if (voice->id() == voicesToTurnOff[i]) {
                    voice->triggerOff(); // TODO use offset for turn off
                }
                voice = voice->next;

            }
        }
    }

    // size_t read(char * dst, size_t sz);

    // Render active voices
    auto voice = mActiveVoices;
    while (voice) {
        if (voice->active()) {
            io.frame(voice->getStartOffsetFrames());
            //                int endOffsetFrames = voice->getEndOffsetFrames();
            //                if (endOffsetFrames >= 0) {
            //                    if (endOffsetFrames < io.framesPerBuffer()) {
            //                        voice->triggerOff(endOffsetFrames);
            //                    }
            //                    endOffsetFrames -= io.framesPerBuffer();
            //                }
            voice->onProcess(io);
        }
        voice = voice->next;
    }
    // Run post processing callbacks
    for (auto cb: mPostProcessing) {
        io.frame(0);
        cb->onAudioCB(io);
    }
    // Move inactive voices to free queue
    if (mMasterMode == TIME_MASTER_AUDIO) {
        if (mFreeVoiceLock.try_lock()) { // Attempt to remove inactive voices without waiting.
            auto voice = mActiveVoices;
            SynthVoice *previousVoice = nullptr;
            while(voice) {
                if (!voice->active()) {
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
}

void PolySynth::render(Graphics &g) {
    if (mMasterMode == TIME_MASTER_GRAPHICS) {
        // FIXME add support for TIME_MASTER_GRAPHICS
    }
    std::unique_lock<std::mutex> lk(mGraphicsLock);
    SynthVoice *voice = mActiveVoices;
    while (voice) {
        if (voice->active()) {
            voice->onProcess(g);
        }
        voice = voice->next;
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

PolySynth &PolySynth::insertBefore(AudioCallback &v, AudioCallback &beforeThis) {
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

void PolySynth::print() {
    {
        std::unique_lock<std::mutex> lk(mFreeVoiceLock);
        auto voice = mFreeVoices;
        int counter = 0;
        std::cout << " ---- Free Voices ----" << std:: endl;
        while(voice) {
            std::cout << "Voice " << counter++ << " " << voice->id() << " : " <<  typeid(voice).name() << " " << voice << std::endl;
            voice = voice->next;
        }
    }
    //
    {
        auto voice = mActiveVoices;
        int counter = 0;
        std::cout << " ---- Active Voices ----" << std:: endl;
        while(voice) {
            std::cout << "Voice " << counter++ << " " << voice->id() << " : " <<  typeid(voice).name() << " " << voice  << std::endl;
            voice = voice->next;
        }
    }
    //
    {
        std::unique_lock<std::mutex> lk(mVoiceToInsertLock);
        auto voice = mVoicesToInsert;
        int counter = 0;
        std::cout << " ---- Queued Voices ----" << std:: endl;
        while(voice) {
            std::cout << "Voice " << counter++ << " " << voice->id() << " : " <<  typeid(voice).name() << " " << voice  << std::endl;
            voice = voice->next;
        }
    }
}

// ---

void SynthSequencer::render(AudioIOData &io) {
    if (mMasterMode ==  PolySynth::TIME_MASTER_AUDIO) {
        double timeIncrement = io.framesPerBuffer()/(double) io.framesPerSecond();
        double blockStartTime = mMasterTime;
        mMasterTime += timeIncrement;
        processEvents(blockStartTime, io.framesPerSecond());

    }
    mPolySynth.render(io);
}

void SynthSequencer::render(Graphics &g) {
    if (mMasterMode == PolySynth::TIME_MASTER_GRAPHICS) {
        double timeIncrement = 1.0/mFps;
        double blockStartTime = mMasterTime;
        mMasterTime += timeIncrement;
        processEvents(blockStartTime, mFps);
    }
    mPolySynth.render(g);
}

void SynthSequencer::processEvents(double blockStartTime, double fps) {
    if (mNextEvent < mEvents.size()) {
        auto iter = mEvents.begin();
        std::advance(iter, mNextEvent);
        auto event = *iter;
        while (event.startTime <= mMasterTime) {
            event.offsetCounter = (event.startTime - blockStartTime)*fps;
            mPolySynth.triggerOn(event.voice, event.offsetCounter);
            //                    std::cout << "Event " << mNextEvent << " " << event.startTime << " " << typeid(*event.voice.get()).name() << std::endl;
            mNextEvent++;
            iter++;
            if (iter == mEvents.end()) {
                break;
            }
            event = *iter;
        }
    }
}
