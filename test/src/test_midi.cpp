
#include "al/io/al_MIDI.hpp"
#include "catch.hpp"

using namespace al;

#ifndef TRAVIS_BUILD

TEST_CASE("MIDI test") {
  RtMidiIn midiInput;
  RtMidiOut midiOutput;
}

#endif
