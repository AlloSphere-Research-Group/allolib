/*

*/

#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"

#include "al/ui/al_ControlGUI.hpp"
#include <iostream>
#include <cstdlib>

using namespace al;
using namespace std;

struct MyApp: App {

  Parameter gain {"gain", "", 0.1f, "", 0.0f, 1.0f};
  Parameter durSecs {"duration", "", 0.4f, "", 0.05, 1.0}; // duration of each burst

  ControlGUI gui;

  int count; // sample counter to determine when to switch speaker
  int numSamps; // number of samples to play on each speaker
  int curOutput; // current speaker index playing

  void onCreate() override {
    gui << gain << durSecs;
    gui.init();
    durSecs.registerChangeCallback([&](float value) {numSamps = audioIO().framesPerSecond() * durSecs;});
  }

  void onDraw(Graphics &g) override {
    g.clear();
    gui.draw(g);
  }

  void onSound(AudioIOData& io) override {
    while (io()) {
      float noise = rnd::uniformS();
      io.out(curOutput) = gain * noise;
      count++;
      if (count == numSamps) {
        count = 0;
        curOutput++;
        curOutput = curOutput%io.channelsOut();
        std::cout << "Playing on Channel " << curOutput << std::endl;
      }
    }
  }
};



int main ()
{
    MyApp app;
    app.initAudio();
    app.start();
    return 0;
}
