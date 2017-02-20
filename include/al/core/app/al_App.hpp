#pragma once

#include "al/core/app/al_WindowApp.hpp"
#include "al/core/app/al_AudioApp.hpp"
#include "al/core/protocol/al_OSC.hpp"

namespace al {

class App: public WindowApp, public AudioApp, public osc::PacketHandler {
public:
  virtual void start() override {
    open(); // windowapp (calls onCreate)
    begin(); // audioapp (only actually begins of initAudio was called before)
    startFPS(); // windowapp (FPS)
    while (!shouldQuit()) {
      // user can quit this loop with windowapp::quit() or clicking close button
      // or with stdctrl class input (ctrl+q)
      loop(dt()); // windowapp (onAnimate, onDraw)
      tickFPS(); // windowapp (FPS)
    }
    close(); // windowapp (calls onExit)
    end(); // audioapp
  }
};

}