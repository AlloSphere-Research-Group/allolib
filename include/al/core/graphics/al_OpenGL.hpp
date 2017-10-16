#ifndef INCLUDE_AL_OPENGL_HPP
#define INCLUDE_AL_OPENGL_HPP

#include "al/core/graphics/al_GLEW.hpp"
#include "al/core/types/al_Color.hpp"

namespace al {

namespace gl {

/// Get current GPU error string
/// \returns the error string or an empty string if no error
///
const char * errorString(bool verbose=false);

/// Print current GPU error state

/// @param[in] msg    Custom error message
/// @param[in] ID   Graphics object ID (-1 for none)
/// \returns whether there was an error
bool error(const char *msg="", int ID=-1);


/// Returns number of bytes for given data type
int numBytes(GLenum v);

/// Get DataType associated with a basic C type
template<typename Type>
GLenum toDataType();

} // gl::

/*!
  \def AL_GRAPHICS_ERROR(msg, ID)
  Used for reporting graphics errors from source files
*/

//#define AL_ENABLE_DEBUG

#ifdef AL_ENABLE_DEBUG
#define AL_GRAPHICS_ERROR(msg, ID) al::gl::error(msg, ID)
#else
#define AL_GRAPHICS_ERROR(msg, ID) ((void)0)
#endif

} // al::

#endif