#include "al/core/graphics/al_OpenGL.hpp"
#include "al/core/system/al_Printing.hpp"

namespace al {

namespace gl {

const char * errorString(bool verbose) {
	GLenum err = glGetError();
	#define CS(GL_ERR, desc) case GL_ERR: return verbose ? #GL_ERR ", " desc : #GL_ERR;
	switch(err){
		case GL_NO_ERROR: return "";
		CS(GL_INVALID_ENUM, "An unacceptable value is specified for an enumerated argument.")
		CS(GL_INVALID_VALUE, "A numeric argument is out of range.")
		CS(GL_INVALID_OPERATION, "The specified operation is not allowed in the current state.")
		CS(GL_INVALID_FRAMEBUFFER_OPERATION, "The framebuffer object is not complete.")
		CS(GL_OUT_OF_MEMORY, "There is not enough memory left to execute the command.")
		default: return "Unknown error code.";
	}
	#undef CS
}

bool error(const char *msg, int ID) {
  const char * errStr = errorString();
  if(errStr[0]){
    if(ID>=0) AL_WARN_ONCE("Error %s (id=%d): %s", msg, ID, errStr);
    else    AL_WARN_ONCE("Error %s: %s", msg, errStr);
    return true;
  }
  return false;
}

int numBytes(GLenum v)
{
  #define CS(a,b) case a: return sizeof(b);
  switch(v){
    CS(GL_BYTE, GLbyte)
    CS(GL_UNSIGNED_BYTE, GLubyte)
    CS(GL_SHORT, GLshort)
    CS(GL_UNSIGNED_SHORT, GLushort)
    CS(GL_INT, GLint)
    CS(GL_UNSIGNED_INT, GLuint)
    CS(GL_FLOAT, GLfloat)
    CS(GL_DOUBLE, GLdouble)
    default: return 0;
  };
  #undef CS
}

template<> GLenum toDataType<char>(){ return GL_BYTE; }
template<> GLenum toDataType<unsigned char>(){ return GL_UNSIGNED_BYTE; }
template<> GLenum toDataType<short>(){ return GL_SHORT; }
template<> GLenum toDataType<unsigned short>(){ return GL_UNSIGNED_SHORT; }
template<> GLenum toDataType<int>(){ return GL_INT; }
template<> GLenum toDataType<unsigned int>(){ return GL_UNSIGNED_INT; }
template<> GLenum toDataType<float>(){ return GL_FLOAT; }
template<> GLenum toDataType<double>(){ return GL_DOUBLE; }

} // gl::

}