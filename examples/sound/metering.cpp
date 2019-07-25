// OutputMaster example
// By Andrés Cabrera mantaraya36@gmail.com
// July 2018

#include <iostream>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/sound/al_OutputMaster.hpp"

// This example shows usage of the OutputMaster class to do
// metering.

using namespace al;

struct MyApp : public App {

  OutputMaster mOutputMaster {2,  44100};

  void onCreate() override {
    // Initialize
    mOutputMaster.initialize(audioIO().channelsOut(), audioIO().framesPerSecond());
    // Enable metering of output
    mOutputMaster.setMeterOn(true);
    // Choose how often the meter values are updated
    mOutputMaster.setMeterUpdateFreq(10);
    // You can append OutputMaster to audioIO()
    // OutputMaster will be run automatically after the onSound() callback
    audioIO().append(mOutputMaster);
  }

  void onSound(AudioIOData &io) override {
    while(io()) {
      io.out(0) = io.in(0)* 0.1f;
      io.out(1) = io.in(1)* 0.1f;
    }
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    float values[2];
    // Copies the current values to the array passed
    mOutputMaster.getCurrentValues(values);
    Mesh m;
    addQuad(m, 0.2f, 0.2f);

    g.color(1.0);
    g.pushMatrix();
    g.translate(-0.5, values[0]* 2, -2);
    g.draw(m);
    g.popMatrix();
    g.pushMatrix();
    g.translate(0.5, values[1]* 2, -2);
    g.draw(m);
    g.popMatrix();

  }

};

int main(int argc, char *argv[])
{
  MyApp app;
  app.dimensions(640, 480);
  app.title("Stereo Audio Scene");
  app.fps(30);
  app.initAudio(44100, 256, 2, 2);
  app.start();
  return 0;
}

