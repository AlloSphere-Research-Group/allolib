#ifndef INCLUDE_AL_GLEW_HPP
#define INCLUDE_AL_GLEW_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "GL/glew.h"

namespace al {

namespace glew {

// load OpenGL functions
// crashes if failed, should be called after context creation
void init();

}

namespace gl
{

bool loaded();

}

}

#endif