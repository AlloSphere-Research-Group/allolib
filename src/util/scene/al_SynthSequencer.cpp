
#include "al/util/scene/al_SynthSequencer.hpp"

using namespace al;

void SynthSequencer::render(AudioIOData &io) {
    if (mMasterMode ==  PolySynth::TIME_MASTER_AUDIO) {
        double timeIncrement = mNormalizedTempo * io.framesPerBuffer()/(double) io.framesPerSecond();
        double blockStartTime = mMasterTime;
        mMasterTime += timeIncrement;
        processEvents(blockStartTime, mNormalizedTempo * io.framesPerSecond());
    }
    mPolySynth->render(io);
}

void SynthSequencer::render(Graphics &g) {
    if (mMasterMode == PolySynth::TIME_MASTER_GRAPHICS) {
        double timeIncrement = 1.0/mFps;
        double blockStartTime = mMasterTime;
        mMasterTime += timeIncrement;
        processEvents(blockStartTime, mNormalizedTempo * mFps);
    }
    mPolySynth->render(g);
}

void SynthSequencer::processEvents(double blockStartTime, double fpsAdjusted) {
    if (mEventLock.try_lock()) {
        if (mNextEvent < mEvents.size()) {
            auto iter = mEvents.begin();
            std::advance(iter, mNextEvent);
            auto event = iter;
            while (event->startTime <= mMasterTime) {
                event->offsetCounter = (event->startTime - blockStartTime)*fpsAdjusted;
                if (event->type == SynthSequencerEvent::EVENT_VOICE) {
                    mPolySynth->triggerOn(event->voice, event->offsetCounter);
                    event->voice = nullptr; // Voice has been consumed
                } else if (event->type == SynthSequencerEvent::EVENT_PFIELDS){
                    auto *voice = mPolySynth->getVoice(event->fields.name);
                    if (voice) {
                        voice->setParamFields(event->fields.pFields);
                        mPolySynth->triggerOn(voice, event->offsetCounter);
                        event->voice = voice;
                    } else {
                        std::cerr << "SynthSequencer::processEvents: Could not get free voice for sequencer!" << std::endl;
                    }
                } else if (event->type == SynthSequencerEvent::EVENT_TEMPO){
                    // TODO support tempo events
                }
//                std::cout << "Event " << mNextEvent << " " << event->startTime << " " << typeid(*event->voice).name() << std::endl;
                mNextEvent++;
                iter++;
                if (iter == mEvents.end()) {
                    break;
                }
                event = iter;
            }
        }
        for (auto &event : mEvents) {
//            if (event.startTime*mNormalizedTempo > mMasterTime) {
//                break;
//            }
            double eventTermination = event.startTime + event.duration;
            if (event.voice && event.voice->active() && eventTermination <= mMasterTime) {
                mPolySynth->triggerOff(event.voice->id());
//                std::cout << "trigger off " <<  event.voice->id() << " " << eventTermination << " " << mMasterTime  << std::endl;
                event.voice = nullptr; // When an event gives up a voice, it is done.
            }
        }
        mEventLock.unlock();
    }
}
