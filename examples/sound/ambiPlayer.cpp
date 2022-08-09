/*

An agent orbits around the origin emitting the audio line input. The camera
view can be switched between a freely navigable keyboard/mouse controlled mode
and a sphere follow mode.

Requirements:
2 channels of spatial sound
2 windows, one front view, one back view
stereographic rendering
*/

#include <cassert>
#include <iostream>

#include "al/app/al_App.hpp"
#include "al/sound/al_Ambisonics.hpp"
#include "al/sound/al_SoundFile.hpp"
#include "al/sphere/al_AlloSphereSpeakerLayout.hpp"

using namespace al;

#define AUDIO_BLOCK_SIZE 512

struct MyApp : public App {
  AmbisonicsSpatializer spatializer;
  SoundFile sf;
  SoundFilePlayer sfPlayer;
  float read_buffer[AUDIO_BLOCK_SIZE * 4];
  float gain;
  int done;
  Speakers speakerLayout;

  void onInit() override {
    // Create spatializer
    //        spatializer = new AmbisonicsSpatializer(speakerLayout, 2, 1);
    spatializer.configure(2, 1, 1);
    spatializer.setSpeakerLayout(speakerLayout);
    spatializer.numFrames(AUDIO_BLOCK_SIZE);

    gain = 1;
    done = 0;

    std::string path = "/Users/create/code/spatial_andres/";
    std::string filename = "Ambisonics/Bees/T08.WAV";

    if (!sf.open((path + filename).c_str())) {
      std::cout << " Can't open file: " << filename << std::endl;
      quit();
    } else {
      std::cout << "Playing file: " << filename << std::endl;
    }

    if (sf.channels != 4) {
      std::cout << "ERROR: Wrong number of channels in file. Expected 4."
                << std::endl;
      quit();
    }
    sfPlayer.soundFile = &sf;
  }

  void onSound(AudioIOData &io) override {
    int numFrames = io.framesPerBuffer();

    assert(AUDIO_BLOCK_SIZE == numFrames);

    spatializer.prepare(io);

    float *ambiChans = spatializer.ambiChans();

    sfPlayer.getFrames(AUDIO_BLOCK_SIZE, read_buffer, AUDIO_BLOCK_SIZE);

    for (int i = 0; i < 1; i++) {
      for (int j = 0; j < AUDIO_BLOCK_SIZE; j++) {
        int chan = 0;
        ambiChans[j + chan * AUDIO_BLOCK_SIZE] = (read_buffer[j * 4 + chan]);
      }
    }
    spatializer.finalize(io);
  }
};

// #define STEREO

int main(int argc, char *argv[]) {
  // Set speaker layout
  AudioDevice::printAll();

  float sr = 48000;

  MyApp app;

  app.speakerLayout = AlloSphereSpeakerLayoutCompensated();

  app.audioDomain()->configure(sr, 256, 60);

  app.start();

  return 0;
}
