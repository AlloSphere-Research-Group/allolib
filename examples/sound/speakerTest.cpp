/*

*/

#include <cstdlib>
#include <iostream>

#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"

using namespace al;
using namespace std;

// This application sends white noise to speakers.

struct MyApp : App {
  Parameter gain{"gain", "", 0.1f, "", 0.0f, 1.0f};
  Parameter durSecs{"duration", "",  0.4f,
                    "",         0.0, 1.0};  // duration of each burst
  ParameterBool autoChange{"autoChange"};
  Trigger next{"next"};
  ParameterInt curOutput{"Current", "", 0,
                         ""};  // current speaker index playing

  ControlGUI gui;

  int count{0};  // sample counter to determine when to switch speaker
  int numSamps;  // number of samples to play on each speaker

  void onCreate() override {
    gui << gain << autoChange << curOutput << durSecs << next;
    gui.init();
    gui.setTitle("Speaker Test");

    curOutput.max(audioIO().channelsOut() - 1);
    durSecs.registerChangeCallback(
        [&](float value) { numSamps = audioIO().framesPerSecond() * durSecs; });
    durSecs = 0.4f;

    next.registerChangeCallback([&](float value) {
      count = 0;
      int nextOutput = curOutput + 1;
      curOutput = nextOutput % audioIO().channelsOut();
    });
  }

  void onDraw(Graphics& g) override {
    g.clear();
    gui.draw(g);
  }

  void onSound(AudioIOData& io) override {
    while (io()) {
      float noise = rnd::uniformS();
      io.out(curOutput) = gain * noise;
      if (autoChange == 1.0) {
        count++;
        if (count >= numSamps) {
          count = 0;
          int nextCurrent = curOutput + 1;
          curOutput = nextCurrent % io.channelsOut();
          std::cout << "Playing on Channel " << curOutput << std::endl;
        }
      }
    }
  }

  void onExit() override { gui.cleanup(); }
};

int main() {
  MyApp app;
  app.configureAudio();
  app.start();
  return 0;
}
