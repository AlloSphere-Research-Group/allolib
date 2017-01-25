#pragma once

#include "al/core/system/al_Time.hpp"

namespace al {

class FPS {
public:
  al_nsec interval = 16666666ll; // 60 fps
  al_nsec deltaTime;
  al_nsec start_of_loop = 0;
  double mFPSWanted = 60;
  void fps(double f);
  double fpsWanted();
  double fps();
  // get time
  double sec();
  double msec(); // millis
  double dt();
  void startFPS();
  void tickFPS();
};

}