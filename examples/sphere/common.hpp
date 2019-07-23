#ifndef __COMMON__
#define __COMMON__

#include "al/app/al_App.hpp"

struct State {
  al::Color backgroundColor {1.0f, 1.0f, 1.0f, 1.0f};
  al::Pose pose;
};

#endif