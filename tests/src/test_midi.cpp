
#include "al/io/al_MIDI.hpp"
#include "gtest/gtest.h"

using namespace al;

#ifndef TRAVIS_BUILD

TEST(MIDI, Basic) {
  RtMidiIn midiInput;
  RtMidiOut midiOutput;
}

#endif
