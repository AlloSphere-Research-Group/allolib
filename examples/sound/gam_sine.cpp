#include "al/app/al_App.hpp"
#include "Gamma/Oscillator.h"

using namespace al;

struct sine_app : App
{
  gam::Sine<> osc;

  void onCreate() override {
    gam::Domain::master().spu(audioIO().framesPerSecond());
    osc.freq(440);
  }

  void onDraw(Graphics& g) override {
    g.clear(0);
  }

  void onSound(AudioIOData& io) override {
    while (io()) {
      float s = osc(); // Generate next sine wave sample
      s *= 0.2f; // Scale the sample down a bit for output
      io.out(0) = s;
      io.out(1) = s;
    }
  }
};

int main()
{
  sine_app app;
  app.initAudio(44100, 512, 2, 0);
  app.start();
}