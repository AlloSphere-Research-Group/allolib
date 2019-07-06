#include "al/graphics/al_OpenGL.hpp"
#include "al/system/al_Printing.hpp"
#include <iostream>
#include <string>

static bool gl_loaded = false;

bool al::gl::load () {
  if (gl_loaded) return true;

  // GLenum err = glewInit();
  // if (err != GLEW_OK) {
  //   std::string err_msg = "[al::gl::load] Glew Error: ";
  //   err_msg += (const char*)glewGetErrorString(err);
  //   err_msg += '\n';
  //   std::cerr << err_msg;
  //   return false;
  // }

  int result = gladLoadGL();
  if (!result)
  {
    std::cerr << "failed loading opengl functions\n";
  }

  gl_loaded = true;
  return true;
}

bool al::gl::loaded () {
  return gl_loaded;
}

const char* al::gl::versionString () {
  // return (const char*)glewGetString(GLEW_VERSION);

  int major = GLVersion.major;
  int minor = GLVersion.minor;

  if (major == 3 && minor == 2) return "OpenGL 3.2";
  if (major == 3 && minor == 3) return "OpenGL 3.3";
  if (major == 4 && minor == 0) return "OpenGL 4.0";
  if (major == 4 && minor == 1) return "OpenGL 4.1";
  if (major == 4 && minor == 2) return "OpenGL 4.2";
  if (major == 4 && minor == 3) return "OpenGL 4.3";
  if (major == 4 && minor == 4) return "OpenGL 4.4";
  if (major == 4 && minor == 5) return "OpenGL 4.5";
  if (major == 4 && minor == 6) return "OpenGL 4.6";

  return "OpenGL unknown version";
}

const char * al::gl::errorString(bool verbose) {
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

bool al::gl::error(const char *msg, int ID) {
  const char * errStr = errorString();
  if(errStr[0]){
    if(ID>=0) AL_WARN_ONCE("Error %s (id=%d): %s", msg, ID, errStr);
    else    AL_WARN_ONCE("Error %s: %s", msg, errStr);
    return true;
  }
  return false;
}

int al::gl::numBytes(GLenum v)
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

template<> GLenum al::gl::toDataType<char>(){ return GL_BYTE; }
template<> GLenum al::gl::toDataType<unsigned char>(){ return GL_UNSIGNED_BYTE; }
template<> GLenum al::gl::toDataType<short>(){ return GL_SHORT; }
template<> GLenum al::gl::toDataType<unsigned short>(){ return GL_UNSIGNED_SHORT; }
template<> GLenum al::gl::toDataType<int>(){ return GL_INT; }
template<> GLenum al::gl::toDataType<unsigned int>(){ return GL_UNSIGNED_INT; }
template<> GLenum al::gl::toDataType<float>(){ return GL_FLOAT; }
template<> GLenum al::gl::toDataType<double>(){ return GL_DOUBLE; }
