#include <stdio.h>

#include "al/core/system/al_Printing.hpp"
#include "al/core/gl/al_Graphics.hpp"

namespace al{

const char * Graphics::errorString(bool verbose){
  GLenum err = glGetError();
  #define CS(GL_ERR, desc) case GL_ERR: return verbose ? #GL_ERR ", " desc : #GL_ERR;
  switch(err){
    case GL_NO_ERROR: return "";
    CS(GL_INVALID_ENUM, "An unacceptable value is specified for an enumerated argument.")
    CS(GL_INVALID_VALUE, "A numeric argument is out of range.")
    CS(GL_INVALID_OPERATION, "The specified operation is not allowed in the current state.")
  #ifdef GL_INVALID_FRAMEBUFFER_OPERATION
    CS(GL_INVALID_FRAMEBUFFER_OPERATION, "The framebuffer object is not complete.")
  #endif
    CS(GL_OUT_OF_MEMORY, "There is not enough memory left to execute the command.")
    CS(GL_STACK_OVERFLOW, "This command would cause a stack overflow.")
    CS(GL_STACK_UNDERFLOW, "This command would cause a stack underflow.")
  #ifdef GL_TABLE_TOO_LARGE
    CS(GL_TABLE_TOO_LARGE, "The specified table exceeds the implementation's maximum supported table size.")
  #endif
    default: return "Unknown error code.";
  }
  #undef CS
}

bool Graphics::error(const char * msg, int ID){
  const char * errStr = errorString();
  if(errStr[0]){
    if(ID>=0) AL_WARN_ONCE("Error %s (id=%d): %s", msg, ID, errStr);
    else    AL_WARN_ONCE("Error %s: %s", msg, errStr);
    return true;
  }
  return false;
}


void Graphics::viewport(int x, int y, int width, int height) {
  glViewport(x, y, width, height);
  //enable(SCISSOR_TEST);
  //glScissor(x, y, width, height);
}

Viewport Graphics::viewport() const {
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  return Viewport(vp[0], vp[1], vp[2], vp[3]);
}

} // al::
