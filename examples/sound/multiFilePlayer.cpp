/*
What is our "Hello world!" app?

An agent orbits around the origin emitting the audio line input. The camera
view can be switched between a freely navigable keyboard/mouse controlled mode
and a sphere follow mode.

Requirements:
2 channels of spatial sound
2 windows, one front view, one back view
stereographic rendering
*/

#include <iostream>
#include <vector>

#include "al/app/al_App.hpp"
#include "al/sound/al_SoundFile.hpp"
#include "al/sound/al_Speaker.hpp"

using namespace al;
using namespace std;

#define AUDIO_BLOCK_SIZE 512

typedef struct {
  float *values;
  int counter;
  int numblocks;
} meters_t;

struct MyApp : public App {
  SoundFile sfs[4];
  float read_buffer[AUDIO_BLOCK_SIZE];
  vector<int> outputMap;

  void onSound(AudioIOData &io) override {
    int numFrames = io.framesPerBuffer();

    int framesRead;
    for (int i = 0; i < 4; i++) {
      framesRead = sfs[i].read(ud->read_buffer, numFrames);
      for (int j = 0; j < framesRead; j++) {
        io.out(outputMap[i], j) += (read_buffer[j] * ud->gain);
      }
    }
  }
};

void audioCB() {}

//#define STEREO

int main(int argc, char *argv[]) {
  float sr = 44100;

  std::vector<std::string> filenames;
  filenames.push_back("data/count.wav");
  filenames.push_back("data/count.wav");
  filenames.push_back("data/count.wav");
  filenames.push_back("data/count.wav");

  for (int i = 0; i < filenames.size(); i++) {
    app.sfs[i].load(filenames[i]);
    if (!ud.sfs[i].openRead()) {
      std::cout << " Can't open file: " << filenames[i] << std::endl;
      return -1;
    } else {
      std::cout << "Playing file: " << filenames[i] << std::endl;
    }
    // TODO check sampling rate;
  }

  MyApp app;
  app.audioDomain()->audioIO().gain(0.5);  // Global output gain.
  app.audioDomain()->configureAudio(44100, AUDIO_BLOCK_SIZE);

  // Map files to speakers
  app.outputMap.resize(filenames.size());

  app.outputMap[0] = 2 - 1;
  app.outputMap[1] = 53 - 1;
  app.outputMap[2] = 59 - 1;
  app.outputMap[3] = 8 - 1;

  ap.start();
  return 0;
}
