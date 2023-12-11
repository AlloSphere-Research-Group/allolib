#include "al/app/al_FPS.hpp"

#include <algorithm> // max
#include <iostream>

using namespace al;

void FPS::fps(double f) {
  mFPSWanted = f;
  interval = static_cast<al_nsec>(al_time_s2ns / f);
}

double FPS::fpsWanted() { return mFPSWanted; }

double FPS::fps() { return al_time_s2ns / deltaTime; }

double FPS::sec() { return al_steady_time_nsec() * al_time_ns2s; }

double FPS::msec() { return al_steady_time_nsec() * 1.0e-6; }

double FPS::dt() { return static_cast<double>(deltaTime); }

double FPS::dt_sec() { return static_cast<double>(deltaTime) * 1.0e-9; }

void FPS::startFPS() {
  deltaTime = interval;
  al_start_steady_clock();
  start_of_loop = 0;
}

void FPS::tickFPS() {
  al_nsec after_loop = al_steady_time_nsec();
  al_nsec time_took_to_loop = after_loop - start_of_loop;
  if (time_took_to_loop < interval) {
    // have some time left
    al_sleep_nsec(interval - time_took_to_loop);
    deltaTime = interval;
    start_of_loop += interval;
  } else {
    // no time to sleep
    deltaTime = time_took_to_loop;
    start_of_loop = after_loop;
  }
}
