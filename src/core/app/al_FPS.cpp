#include "al/core/app/al_FPS.hpp"

#include <algorithm> // max

using namespace al;

void FPS::fps(double f) {
  mFPSWanted = f;
  interval = static_cast<al_nsec>(al_time_s2ns / f);
}

double FPS::fpsWanted() {
  return mFPSWanted;
}

double FPS::fps() {
    return al_time_s2ns / deltaTime;
}

double FPS::sec() {
  return al_steady_time_nsec() * al_time_ns2s;
}

double FPS::msec() {
  return al_steady_time_nsec() * 1.0e-6;
}

double FPS::dt() {
  return static_cast<double>(deltaTime);
}

double FPS::dt_sec() {
  return static_cast<double>(deltaTime) / 1000000000.0;
}

void FPS::startFPS() {
  deltaTime = interval;
  al_start_steady_clock();
}

void FPS::tickFPS() {
    al_nsec after_loop = al_steady_time_nsec();
    al_nsec to_sleep = std::max(interval - (after_loop - start_of_loop), 0ll);
    al_sleep_nsec(to_sleep);
    // dt = new start_of_loop - old start_of_loop
    deltaTime = (after_loop + to_sleep) - start_of_loop;
    start_of_loop = after_loop + to_sleep;
}