// OutputMaster example
// By Andrés Cabrera mantaraya36@gmail.com
// July 2018

#include <iostream>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"

// This example shows a simple input level meter

using namespace al;

struct MyApp : public App {
  float *meterValues;

  void onCreate() override { meterValues = new float(audioIO().channelsIn()); }

  void onSound(AudioIOData &io) override {
    while (io()) {
      for (auto i = 0; i < io.channelsOut(); i++) {
        // Write meter values. This is only safe because float is atomic on
        // desktops.
        if (meterValues[i] < fabs(io.in(0))) {
          meterValues[i] = io.in(0);
        }
      }
    }
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    // Copies the current values to the array passed
    Mesh m;
    addQuad(m, 0.2f, 0.2f);

    g.color(1.0);

    for (auto i = 0; i < audioIO().channelsOut(); i++) {
      g.pushMatrix();
      g.color(HSV(0.5f + 0.5f * meterValues[i]));
      g.translate(-1.0f + (2.0f * i / (audioIO().channelsOut() - 1.0f)),
                  -0.5f + meterValues[i], -4);
      meterValues[i] = 0;
      g.draw(m);
      g.popMatrix();
    }
  }
};

int main(int argc, char *argv[]) {
  MyApp app;
  app.title("Stereo Audio Scene");
  app.fps(30);
  app.configureAudio(44100, 256, 2, 2);
  app.start();
  return 0;
}
