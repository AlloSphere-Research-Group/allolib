/*
Based on Allocore Example: Audio To Graphics by Lance Putnam

*/

#include <iostream>

#include "al/core.hpp"
#include "al/spatialaudio/al_Convolver.hpp"

#include "Gamma/Noise.h"
#include "Gamma/Envelope.h"

#define BLOCK_SIZE 64

using namespace std;
using namespace al;

struct MyApp : public App {
public:
  double phase;
  Convolver conv;
  vector<float *> IRchannels;

  MyApp()
  { }

  ~MyApp() {
  }

  void onInit() {
    // Make IR
    int irdur = 1.0; // duration of the IR in seconds
    int numFrames = audioIO().framesPerSecond() * irdur;
    int numIRChannels = 2; //Number of channels in the IR
    gam::NoisePink<> noise;
    gam::Decay<> env(numFrames);

    // Make two IRs from pink noise and a exponential decay.
    // You could also load the data from a soundfile
    for (int chan = 0; chan < numIRChannels; chan++) {
      float *ir = new  float[numFrames];
      for (int frame = 0; frame < numFrames; frame++) {
        ir[frame] = noise() * env() * 0.1;
      }
      env.reset();
      IRchannels.push_back(ir);
    }
    int numActiveChannels;
    vector<unsigned int> disabledChannels;
    numActiveChannels = numIRChannels;
    if(numActiveChannels != audioIO().channelsOutDevice()){//more outputs than IR channels
      for(unsigned int i = numActiveChannels; i < audioIO().channelsOutDevice(); ++i){
        disabledChannels.push_back(i);
        cout << "Audio channel " << i << " disabled." << endl;
      }
    }

    // Setup convolver
//    unsigned int ioBufferSize,
//                              vector<float *> IRs, uint32_t IRlength,
//                              map<uint32_t, vector<uint32_t>> channelRoutingMap,
//                              uint32_t basePartitionSize, float density,
//                              uint32_t options
    conv.configure(audioIO().framesPerBuffer(), IRchannels, numFrames, {0, {0},  });
    audioIO().append(conv); // By appending, the convolution will be perfomed after any other processes
  }

  //	// Audio callback
  void onSound(AudioIOData& io) override {
    // Nothing needed here as convolution is performed as the last process since it has been
    // "appended" to AudioIO
  }

  void onAnimate(double dt) override {

  }

  void onDraw(Graphics& g) override {

  }

  void onExit() override {

    // Cleanup memory allocation
    for (unsigned int i = 0; i < IRchannels.size(); i++) {
      delete IRchannels[i];
    }
  }

};


int main(){
  int num_chnls = 2;
  double sampleRate = 44100;

  MyApp app;
  app.initAudio(sampleRate, 512, num_chnls, num_chnls);
  app.start();
}
