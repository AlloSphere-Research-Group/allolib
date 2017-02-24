#pragma once

#include "al/core/app/al_WindowApp.hpp"
#include "al/core/app/al_AudioApp.hpp"
#include "al/core/protocol/al_OSC.hpp"

namespace al {

// Unified app class: single window, audioIO,
//   single port osc recv and single port osc send, and al::Graphics

// TODO: better osc interface, add console app and graphics
class App: public WindowApp, public AudioApp, public osc::PacketHandler {
public:
  virtual void onAnimate(double dt) {}
  virtual void onExit() {}

  // overrides WindowApp's start
  virtual void start() override {
    open(); // WindowApp (calls onCreate)
    begin(); // AudioApp (only actually begins of initAudio was called before)
    startFPS(); // WindowApp (FPS)
    while (!shouldQuit()) {
      // user can quit this loop with WindowApp::quit() or clicking close button
      // or with stdctrl class input (ctrl+q)

      // dt from WindowApp's FPS
      // TODO: audiorate?
      double dt_ = dt();
      onAnimate(dt_);
      loop(); // WindowApp (onDraw)
      tickFPS(); // WindowApp (FPS)
    }
    onExit();
    close(); // WindowApp (calls onExit)
    end(); // AudioApp
  }

  // PacketHandler
  virtual void onMessage(osc::Message& m) override {}
};

}