
#include <cstdio>
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstring>

#include "al/soundfile/al_SoundfileBufferedRecord.hpp"
#include "al/core/io/al_AudioIO.hpp"

#include "Gamma/SoundFile.h"

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#define BLOCK_SIZE 64 //min 64, max 8192

using namespace std;

TEST_CASE( "Mono recorder", "[SoundFileBufferedRecord]" ) {
  al::SoundFileBufferedRecord soundFile;

  soundFile.open("output.wav", 44100, 1);

  float buffer[256];
  for (int i = 0 ; i < 256; i++) {
      buffer[i] = float(i)/256;
  }
  soundFile.write({buffer}, 256);
  this_thread::sleep_for(chrono::milliseconds(10));
  soundFile.write({buffer}, 256);
  this_thread::sleep_for(chrono::milliseconds(10));
  soundFile.write({buffer}, 256);
  this_thread::sleep_for(chrono::milliseconds(10));
  soundFile.write({buffer}, 256);
  this_thread::sleep_for(chrono::milliseconds(10));
  soundFile.write({buffer}, 256);
  this_thread::sleep_for(chrono::milliseconds(10));
  soundFile.write({buffer}, 256);
  this_thread::sleep_for(chrono::milliseconds(10));
  soundFile.write({buffer}, 256);
  this_thread::sleep_for(chrono::milliseconds(10));
  soundFile.write({buffer}, 256);
  this_thread::sleep_for(chrono::milliseconds(10));
  soundFile.close();

  // Now read file back

  gam::SoundFile sf("output.wav");
  sf.openRead();

  float writtenbuffer[256];
  for (int j = 0; j < 8; j++) {
    int read = sf.read<float>(writtenbuffer, 256);
    REQUIRE(read == 256);
    for (int i = 0 ; i < 256; i++) {
        REQUIRE(fabs(writtenbuffer[i] - buffer[i]) < 0.00001f);
    }
  }
}
