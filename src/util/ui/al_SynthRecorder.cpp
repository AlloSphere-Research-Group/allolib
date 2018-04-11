
#include "al/util/ui/al_SynthRecorder.hpp"

using namespace al;

void SynthRecorder::startRecord(std::string name, bool overwrite, bool startOnEvent) {
    mOverwrite = overwrite;
    mRecording = true;
    mStartOnEvent = startOnEvent;
    mSequenceStart = std::chrono::high_resolution_clock::now();
    mSequenceName = name;
}

void SynthRecorder::stopRecord() {
    mRecording = false;
    std::string path = ".";
    if (path.back() != '/') {
        path += "/";
    }
    std::string fileName = path + mSequenceName + ".synthSequence";

    std::string newSequenceName = mSequenceName;
    if (!mOverwrite) {
        std::string newFileName = fileName;
        int counter = 0;
        while (File::exists(newFileName)) {
            newSequenceName = mSequenceName + "_" + std::to_string(counter++);
            newFileName =  path + newSequenceName + ".synthSequence";
        }
        fileName = newFileName;
    }
    std::ofstream f(fileName);
    if (!f.is_open()) {
        std::cout << "Error while opening sequence file: " << fileName << std::endl;
        return;
    }
    if (mFormat == CPP_FORMAT) {
        for (SynthEvent event: mSequence) {
            f << "s.add<" << event.synthName << ">("  << event.time << ").set(";
            for (unsigned int i = 0; i < event.pFields.size(); i++) {
                f <<  event.pFields[i];
                if (i <  event.pFields.size() - 1) {
                    f << ", ";
                }
            }
            f << ");" << std::endl;
        }
    } else if (mFormat == SEQUENCER_FORMAT) {
        for (SynthEvent event: mSequence) {
            if (event.type == SynthEventType::NOTE) {
                f << "@ " << event.time << " " << event.synthName << " ";
                for (unsigned int i = 0; i < event.pFields.size(); i++) {
                    f <<  event.pFields[i] << " ";
                }
                f << std::endl;
            }
            if (event.type == SynthEventType::TRIGGER_ON) {
                f << "+ " << event.time << " " << event.id << " " << event.synthName << " ";
                for (unsigned int i = 0; i < event.pFields.size(); i++) {
                    f <<  event.pFields[i] << " ";
                }
                f << std::endl;
            }
            if (event.type == SynthEventType::TRIGGER_OFF) {
                f << "- " << event.time << " " << event.id << " ";
                for (unsigned int i = 0; i < event.pFields.size(); i++) {
                    f <<  event.pFields[i] << " ";
                }
                f << std::endl;
            }
        }
    }

    if (f.bad()) {
        std::cout << "Error while writing sequence file: " << fileName << std::endl;
    }
    f.close();

    std::cout << "Recorded: " << fileName << std::endl;
    //        recorder->mLastSequenceName = newSequenceName;
    //        recorder->mLastSequenceSubDir = recorder->mPresetHandler->getSubDirectory();
}

void SynthRecorder::registerPolySynth(PolySynth &polySynth) {
    polySynth.registerTriggerOnCallback(SynthRecorder::onTriggerOn, this);
    polySynth.registerTriggerOffCallback(SynthRecorder::onTriggerOff, this);
    mPolySynth = &polySynth;
}


