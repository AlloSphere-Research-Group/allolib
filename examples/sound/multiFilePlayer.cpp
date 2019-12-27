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
  uint64_t frameCounter = 0;
  vector<uint16_t> outputMap;

  void onSound(AudioIOData &io) override {
    uint64_t numFrames = io.framesPerBuffer();
    if (frameCounter + numFrames > sfs[0].frameCount) {
      numFrames = sfs[0].frameCount - frameCounter;
    }
    std::cout << frameCounter << std::endl;
    for (uint16_t i = 0; i < 4; i++) {
      float *frames = sfs[i].getFrame(frameCounter);
      memcpy(io.outBuffer(i), frames, numFrames * sizeof(float));
    }
    frameCounter += io.framesPerBuffer();
    if (frameCounter >= sfs[0].frameCount) {
      frameCounter = 0;
    }
  }
};

int main() {
  std::vector<std::string> filenames;
  filenames.push_back("data/count.wav");
  filenames.push_back("data/count.wav");
  filenames.push_back("data/count.wav");
  filenames.push_back("data/count.wav");

  MyApp app;

  app.outputMap = {0, 1, 2, 3};
  //  app.outputMap[0] = 2 - 1;
  //  app.outputMap[1] = 53 - 1;
  //  app.outputMap[2] = 59 - 1;
  //  app.outputMap[3] = 8 - 1;

  for (size_t i = 0; i < filenames.size(); i++) {
    if (!app.sfs[i].open(filenames[i].c_str())) {
      std::cout << " Can't open file: " << filenames[i] << std::endl;
      return -1;
    } else {
      std::cout << "Playing file " << i << " : " << filenames[i] << std::endl;
      std::cout << " -- sr: " << app.sfs[i].sampleRate
                << " total frames: " << app.sfs[i].frameCount
                << " channels: " << app.sfs[i].channels << std::endl;
    }
  }

  float sr = app.sfs[0].sampleRate;
  app.audioDomain()->audioIO().gain(0.5);  // Global output gain.
  app.audioDomain()->configure(sr, AUDIO_BLOCK_SIZE, 4);
  app.start();
  return 0;
}
