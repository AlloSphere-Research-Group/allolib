#ifndef __COMMON__
#define __COMMON__

#include "al/core.hpp"

struct State {
  al::Color backgroundColor {1.0f, 1.0f, 1.0f, 1.0f};
  al::Pose pose;
};

#endif