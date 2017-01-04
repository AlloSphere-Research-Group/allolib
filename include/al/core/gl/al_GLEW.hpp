#pragma once

#ifdef AL_WINDOWS
#define GLEW_STATIC
#endif
#include "GL/glew.h"

namespace al {

namespace glew {

// load OpenGL functions
// crashes if failed, should be called after context creation
void init();

}

}