#include "al/ui/al_ParameterMIDI.hpp"

using namespace al;

ParameterMIDI::ParameterMIDI(unsigned int deviceIndex, bool verbose) {
  MIDIMessageHandler::bindTo(mRtMidiIn);
  mVerbose = verbose;
  try {
    mRtMidiIn.openPort(deviceIndex);
    printf("ParameterMIDI: Opened port to %s\n",
           mRtMidiIn.getPortName(deviceIndex).c_str());
  } catch (RtMidiError &error) {
    std::cout << "ParameterMIDI Warning opening MIDI port:"
              << error.getMessage() << deviceIndex << std::endl;
  }
}

void ParameterMIDI::init(int deviceIndex, bool verbose) {
  open(deviceIndex, verbose);
}

void ParameterMIDI::open(unsigned int deviceIndex) {
  MIDIMessageHandler::bindTo(mRtMidiIn);
  try {
    mRtMidiIn.openPort(deviceIndex);
    printf("ParameterMIDI: Opened port to %s\n",
           mRtMidiIn.getPortName(deviceIndex).c_str());
  } catch (RtMidiError &error) {
    std::cout << "ParameterMIDI Warning opening MIDI port:"
              << error.getMessage() << deviceIndex << std::endl;
  }
}

void ParameterMIDI::open(int deviceIndex, bool verbose) {
  mVerbose = verbose;
  if (deviceIndex >= 0) {
    open(deviceIndex);
  }
}

void ParameterMIDI::close() {
  mRtMidiIn.closePort();
  MIDIMessageHandler::clearBindings();
}

void ParameterMIDI::connectControl(Parameter &param, int controlNumber,
                                   int channel, float min, float max) {
  ControlBinding newBinding;
  newBinding.controlNumber = controlNumber;
  newBinding.channel = channel - 1;
  newBinding.param = &param;
  newBinding.min = min;
  newBinding.max = max;
  mControlBindings.push_back(newBinding);
}

void ParameterMIDI::connectControls(ParameterMeta &param,
                                    std::vector<int> controlNumbers,
                                    int channel, std::vector<float> min,
                                    std::vector<float> max) {
  AbstractBinding newBinding;
  newBinding.controlNumbers = controlNumbers;
  newBinding.channel = channel - 1;
  newBinding.param = &param;
  std::vector<VariantValue> fields;
  param.getFields(fields);
  if (fields.size() < controlNumbers.size()) {
    controlNumbers.resize(fields.size());
    std::cout << "WARNING: connectPoseControl() resizing control numbers to "
                 "match fields"
              << std::endl;
  }
  if (min.size() < controlNumbers.size()) {
    for (size_t i = max.size(); i < controlNumbers.size(); i++) {
      min.push_back(0.0);
    }
  }
  if (max.size() < controlNumbers.size()) {
    for (size_t i = max.size(); i < controlNumbers.size(); i++) {
      max.push_back(1.0);
    }
  }
  newBinding.min = min;
  newBinding.max = max;
  mAbstractBindings.push_back(newBinding);
}

void ParameterMIDI::connectNoteToValue(Parameter &param, int channel, float min,
                                       int low, float max, int high) {
  if (high == -1) {
    max = min;
    high = low;
  }
  for (int num = low; num <= high; ++num) {
    NoteBinding newBinding;
    newBinding.noteNumber = num;
    if (num != high) {
      newBinding.value = min + (max - min) * (num - low) / (high - low);
    } else {
      newBinding.value = max;
    }
    newBinding.channel = channel - 1;
    newBinding.param = &param;
    mNoteBindings.push_back(newBinding);
  }
}

void ParameterMIDI::connectNoteToToggle(ParameterBool &param, int channel,
                                        int note) {
  ToggleBinding newBinding;
  newBinding.noteNumber = note;
  newBinding.toggle = true;
  newBinding.channel = channel - 1;
  newBinding.param = &param;
  mToggleBindings.push_back(newBinding);
}

void ParameterMIDI::connectNoteToIncrement(Parameter &param, int channel,
                                           int note, float increment) {
  IncrementBinding newBinding;
  newBinding.channel = channel - 1;
  newBinding.noteNumber = note;
  newBinding.increment = increment;
  newBinding.param = &param;
  mIncrementBindings.push_back(newBinding);
}

void ParameterMIDI::onMIDIMessage(const MIDIMessage &m) {
  if (m.type() & MIDIByte::CONTROL_CHANGE) {
    for (ControlBinding &binding : mControlBindings) {
      if (m.channel() == binding.channel &&
          m.controlNumber() == binding.controlNumber) {
        float newValue =
            binding.min + (m.controlValue() * (binding.max - binding.min));
        binding.param->set(newValue);
      }
    }
    for (auto &binding : mAbstractBindings) {
      if (m.channel() == binding.channel) {
        auto foundControl =
            std::find(binding.controlNumbers.begin(),
                      binding.controlNumbers.end(), m.controlNumber());
        if (foundControl != binding.controlNumbers.end()) {
          std::vector<VariantValue> currentFields;
          binding.param->getFields(currentFields);
          size_t index =
              std::distance(binding.controlNumbers.begin(), foundControl);
          float newValue =
              binding.min[index] +
              (m.controlValue() * (binding.max[index] - binding.min[index]));
          currentFields[index] = newValue;
          binding.param->setFields(currentFields);
        }
      }
    }
  }
  if (m.type() & MIDIByte::NOTE_ON && m.velocity() > 0) {
    for (NoteBinding &binding : mNoteBindings) {
      if (m.channel() == binding.channel &&
          m.noteNumber() == binding.noteNumber) {
        binding.param->set(binding.value);
      }
    }
    for (IncrementBinding &binding : mIncrementBindings) {
      if (m.channel() == binding.channel &&
          m.noteNumber() == binding.noteNumber) {
        binding.param->set(binding.param->get() + binding.increment);
      }
    }
    for (ToggleBinding &binding : mToggleBindings) {
      if (m.channel() == binding.channel &&
          m.noteNumber() == binding.noteNumber) {
        if (binding.toggle == true) {
          binding.param->set(binding.param->get() == binding.param->max()
                                 ? binding.param->min()
                                 : binding.param->max());
        } else {
          binding.param->set(binding.param->max());
        }
      }
    }
  } else if (m.type() & MIDIByte::NOTE_OFF ||
             (m.type() & MIDIByte::NOTE_ON && m.velocity() == 0)) {
    for (ToggleBinding &binding : mToggleBindings) {
      if (m.channel() == binding.channel &&
          m.noteNumber() == binding.noteNumber) {
        if (binding.toggle != true) {
          binding.param->set(binding.param->min());
        }
      }
    }
  }
  if (mVerbose) {
    m.print();
  }
}
