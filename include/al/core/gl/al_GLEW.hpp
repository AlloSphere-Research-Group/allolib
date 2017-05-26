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

inline void default_gl_settings() {
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1);
}

}

}

#endif