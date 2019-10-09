#include "al/system/al_Time.hpp"
#include <stdio.h>  // printf
#include <chrono>
#include <iomanip>  // std::setw
#include <iostream>
#include <sstream>  // ostringstream
#include <thread>
#include "al/math/al_Constants.hpp"

auto t0(bool reset = false) {
  static auto t = std::chrono::steady_clock::now();
  if (reset) {
    t = std::chrono::steady_clock::now();
  }
  return t;
}

namespace al {

/// Get current wall time in seconds
al_sec al_system_time() { return al_system_time_nsec() * al_time_ns2s; }

al_nsec al_system_time_nsec() {
  return (std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count());
}

void al_start_steady_clock() { t0(); }

void al_reset_steady_clock() { t0(true); }

al_sec al_steady_time() { return al_steady_time_nsec() * al_time_ns2s; }

al_nsec al_steady_time_nsec() {
  return (std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::steady_clock::now() - t0())
              .count());
}

double al_seconds() { return al_steady_time_nsec() * al_time_ns2s; }

/// Sleep for an interval of seconds
void al_sleep(al_sec dt) { al_sleep_nsec(dt * al_time_s2ns); }

void al_sleep_nsec(al_nsec dt) {
  std::this_thread::sleep_for(std::chrono::nanoseconds(dt));
}

void al_sleep_until(al_sec target) {
  al_sec dt = target - al_system_time();
  if (dt > 0) al_sleep(dt);
}

std::string toTimecode(al_nsec t, const std::string& format) {
  unsigned day =
      t / (al_nsec(1000000000) * 60 * 60 * 24);  // basically for overflow
  unsigned hrs = t / (al_nsec(1000000000) * 60 * 60) % 24;
  unsigned min = t / (al_nsec(1000000000) * 60) % 60;
  unsigned sec = t / (al_nsec(1000000000)) % 60;
  unsigned msc = t / (al_nsec(1000000)) % 1000;
  unsigned usc = t / (al_nsec(1000)) % 1000;

  std::ostringstream s;
  s.fill('0');

  for (unsigned i = 0; i < format.size(); ++i) {
    const auto c = format[i];
    switch (c) {
      case 'D':
        s << day;
        break;
      case 'H':
        s << std::setw(2) << hrs;
        break;
      case 'M':
        s << std::setw(2) << min;
        break;
      case 'S':
        s << std::setw(2) << sec;
        break;
      case 'm':
        s << std::setw(3) << msc;
        break;
      case 'u':
        s << std::setw(3) << usc;
        break;
      default:
        s << c;
    }
  }

  return s.str();
}

void Timer::print() const {
  auto t = getTime();
  auto dt = t - mStart;
  double dtSec = al_time_ns2s * dt;
  printf("%g sec (%g ms) elapsed\n", dtSec, dtSec * 1000.);
}

void DelayLockedLoop ::setBandwidth(double bandwidth) {
  double F = 1. / tperiod;  // step rate
  double omega = M_PI * 2.8 * bandwidth / F;
  mB = omega * sqrt(2.);  // 1st-order weight
  mC = omega * omega;     // 2nd-order weight
}

void DelayLockedLoop ::step(al_sec realtime) {
  if (mReset) {
    // The first iteration sets initial conditions.

    // init loop
    t2 = tperiod;
    t0 = realtime;
    t1 = t0 + t2;  // t1 is ideally the timestamp of the next block start

    // subsequent iterations use the other branch:
    mReset = false;
  } else {
    // read timer and calculate loop error
    // e.g. if t1 underestimated, terr will be
    al_sec terr = realtime - t1;
    // update loop
    t0 = t1;               // 0th-order (distance)
    t1 += mB * terr + t2;  // integration of 1st-order (velocity)
    t2 += mC * terr;       // integration of 2nd-order (acceleration)
  }

  //    // now t0 is the current system time, and t1 is the estimated system
  //    time at the next step
  //    //
  //    al_sec tper_estimate = t1-t0;  // estimated real duration between this
  //    step & the next one double factor = tperiod/tper_estimate;  // <1 if we
  //    are too slow, >1 if we are too fast double real_rate = 1./tper_estimate;
  //    al_sec tper_estimate2 = t2;  // estimated real duration between this
  //    step & the next one double factor2 = 1./t2;  // <1 if we are too slow,
  //    >1 if we are too fast printf("factor %f %f rate %f\n", factor, factor2,
  //    real_rate);
}

}  // namespace al
