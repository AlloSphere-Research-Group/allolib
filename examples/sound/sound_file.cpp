#include "al/core.hpp"
#include "Gamma/SoundFile.h"
#include <iostream>
using namespace al;
using namespace std;

struct sine_app : App
{
  const char * path = "data/count.wav";
  gam::SoundFile sf {path};

  void onCreate() override {
    if (sf.openRead()) {
      cout << "count.wav opened" << endl;
    }
    else {
      cout << "failed to open count.wav" << endl;
      quit();
    }
  }

  void onDraw(Graphics& g) override {
    g.clear(0);
  }

  void onSound(AudioIOData& io) override {
    while (io()) {
      io.out(0) = 0;
      io.out(1) = 0;
    }
  }
};

int main()
{
  sine_app app;
  app.initAudio(44100, 512, 2, 0);
  app.start();
}