#ifndef INCLUDE_AL_APP_HPP
#define INCLUDE_AL_APP_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/app/al_WindowApp.hpp"
#include "al/core/app/al_AudioApp.hpp"
#include "al/core/protocol/al_OSC.hpp"

#include "al/core/gl/al_Viewpoint.hpp"
#include "al/core/spatial/al_Pose.hpp"
#include "al/core/io/al_ControlNav.hpp"

namespace al {

// Unified app class: single window, audioIO, and
//   single port osc recv & single port osc send
// TODO: better osc interface
class App: public WindowApp, public AudioApp, public osc::PacketHandler {
public:

  virtual void onAnimate(double dt) {}
  virtual void onExit() {}

  // overrides WindowApp's start to also initiate AudioApp and etc.
  virtual void start() override {
    open(); // WindowApp (glfw::init(), onInit(), create(), onCreate())
    startFPS(); // WindowApp (FPS)
    begin(); // AudioApp (only begins if `initAudio` was called before)
    while (!shouldQuit()) {
      // user can quit this loop with WindowApp::quit() or clicking close button
      // or with stdctrl class input (ctrl+q)
      onAnimate(dt());
      loop(); // WindowApp (onDraw, refresh)
      tickFPS(); // WindowApp (FPS)
    }
    onExit(); // user defined
    end(); // AudioApp
    close(); // WindowApp (calls onExit)
  }

  // PacketHandler
  virtual void onMessage(osc::Message& m) override {}
};

}

#endif