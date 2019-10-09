#ifndef INCLUDE_AL_FPS_HPP
#define INCLUDE_AL_FPS_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
 */

#include "al/system/al_Time.hpp"

namespace al {

/**
 * @brief FPS class
 * @ingroup App
 */
class FPS {
 public:
  al_nsec interval = 16666666ll;  // 60 fps by default
  al_nsec deltaTime;
  al_nsec start_of_loop = 0;
  double mFPSWanted = 60;
  void fps(double f);
  double fpsWanted();
  double fps();
  // get time
  double sec();
  double msec();    // millis
  double dt();      // in nanos
  double dt_sec();  // in seconds
  void startFPS();
  void tickFPS();
};

}  // namespace al

#endif