#include "al/scene/al_SequencerMIDI.hpp"

using namespace al;
SequencerMIDI::SequencerMIDI(int deviceIndex) : mSynth(nullptr) {
  MIDIMessageHandler::bindTo(mRtMidiIn);
  try {
    mRtMidiIn.openPort(deviceIndex);
    printf("PresetMIDI: Opened port to %s\n",
           mRtMidiIn.getPortName(deviceIndex).c_str());
  } catch (RtMidiError & /*error*/) {
    std::cout << "PresetMIDI Warning: Could not open MIDI port " << deviceIndex
              << std::endl;
  }
}

SequencerMIDI::SequencerMIDI(int deviceIndex, PolySynth &synth)
    : SequencerMIDI(deviceIndex) {
  setSynthSequencer(synth);
}

void SequencerMIDI::open(int deviceIndex, PolySynth &synth) {
  open(deviceIndex);
  setSynthSequencer(synth);
}

void SequencerMIDI::open(int deviceIndex) {
  MIDIMessageHandler::bindTo(mRtMidiIn);

  if (mRtMidiIn.isPortOpen()) {
    mRtMidiIn.closePort();
  }
  try {
    if (deviceIndex >= 0 && deviceIndex < (int)mRtMidiIn.getPortCount()) {
      mRtMidiIn.openPort(deviceIndex);
      printf("PresetMIDI: Opened port to %s\n",
             mRtMidiIn.getPortName(deviceIndex).c_str());
    } else {
      std::cerr << "PresetMIDI Warning: Could not open MIDI port "
                << deviceIndex << std::endl;
    }
  } catch (RtMidiError & /*error*/) {
    std::cerr << "PresetMIDI Warning: Could not open MIDI port " << deviceIndex
              << std::endl;
  }
}

void SequencerMIDI::close() {
  mRtMidiIn.closePort();
  MIDIMessageHandler::clearBindings();
}

bool SequencerMIDI::isOpen() { return mRtMidiIn.isPortOpen(); }

void SequencerMIDI::connectNoteOnToFunction(
    std::function<void(int, int, int)> function) {
  mNoteOnFunctions.push_back(function);
}

void SequencerMIDI::connectNoteOffToFunction(
    std::function<void(int, int, int)> function) {
  mNoteOffFunctions.push_back(function);
}

void SequencerMIDI::onMIDIMessage(const MIDIMessage &m) {
  if (m.type() == MIDIByte::NOTE_ON && m.velocity() > 0) {
    for (auto function : mNoteOnFunctions) {
      //				std::cout << binding.channel << " " <<
      // binding.noteNumber << " " << binding.presetIndex << std::endl;
      //				std::cout << (int) m.channel() <<
      // std::endl;
      function(m.noteNumber(), m.velocity(), m.channel());
    }
  } else if (m.type() == MIDIByte::NOTE_OFF ||
             (m.type() == MIDIByte::NOTE_ON && m.velocity() == 0)) {
    for (auto function : mNoteOffFunctions) {
      //				std::cout << binding.channel << " " <<
      // binding.noteNumber << " " << binding.presetIndex << std::endl;
      //				std::cout << (int) m.channel() <<
      // std::endl;
      function(m.noteNumber(), m.velocity(), m.channel());
    }
  } else if (m.type() == MIDIByte::PROGRAM_CHANGE && m.velocity() > 0) {
    //            for(ProgramBinding binding: mProgramBindings) {
    //                //				std::cout <<
    //                binding.channel << " " << binding.noteNumber << " " <<
    //                binding.presetIndex << std::endl;
    //                //				std::cout << (int)
    //                m.channel() << std::endl; if (m.channel() ==
    //                binding.channel
    //                        && m.noteNumber() == binding.programNumber) {
    //                    //					m.print();
    //                    mPresetHandler->recallPreset(binding.presetIndex);
    //                }
    //            }
  } else if (m.type() == MIDIByte::CONTROL_CHANGE) {
    //            if (m.controlNumber() == mMorphBinding.controlNumber
    //                    && m.channel() == mMorphBinding.channel) {
    //                mPresetHandler->setMorphTime(mMorphBinding.min +
    //                m.controlValue() * (mMorphBinding.max -
    //                mMorphBinding.min));
    //            }
  }
}
