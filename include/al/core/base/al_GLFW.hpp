#pragma once

#include "al/core/base/al_GLEW.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace al {

namespace glfw {
    void init();
    void destroy();
}

}