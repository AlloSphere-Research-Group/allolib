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
  void fps(double f); // set target fps
  double fpsWanted(); // target fps
  double fps();       // effective fps
  // get time
  double sec();    // elapsed seconds
  double msec();   // elapsed millis
  double dt();     // in nanos
  double dt_sec(); // in seconds

  void startFPS();
  void tickFPS();

private:
  al_nsec interval = 16666666ll; // 60 fps by default
  al_nsec deltaTime;
  al_nsec start_of_loop = 0;
  double mFPSWanted = 60;
};

} // namespace al

#endif
