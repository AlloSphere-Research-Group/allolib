#ifndef INCLUDE_AL_OPENGL_HPP
#define INCLUDE_AL_OPENGL_HPP

#include "glad/glad.h"

namespace al {

namespace gl {

bool load();
bool loaded();

const char *versionString();

/// Get current GPU error string
/// \returns the error string or an empty string if no error
///
const char *errorString(bool verbose = false);

/// Print current GPU error state

/// @param[in] msg    Custom error message
/// @param[in] ID   Graphics object ID (-1 for none)
/// \returns whether there was an error
bool error(const char *msg = "", int ID = -1);

/// Returns number of bytes for given data type
int numBytes(GLenum v);

/// Get DataType associated with a basic C type
template <typename Type>
GLenum toDataType();

/// buffer=[GL_NONE, GL_FRONT_LEFT, GL_FRONT_RIGHT, GL_BACK_LEFT,
///         GL_BACK_RIGHT, GL_FRONT, GL_BACK, GL_LEFT, GL_RIGHT,
///         GL_FRONT_AND_BACK]
void bufferToDraw(unsigned int buffer);

void viewport(int left, int bottom, int width, int height);

/// Turn blending on/off
void blending(bool doBlend);

/// Set blend mode
/// src,dst=[GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR,
///          GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA,
///          GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
///          GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR,
///          GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA]
/// eq=[GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT,
///     GL_MIN, GL_MAX]
void blendMode(unsigned int src, unsigned int dst, unsigned int eq);

/// Turn depth testing on/off
void depthTesting(bool testDepth);

/// Turn the depth mask on/off
void depthMask(bool maskDepth);

/// Turn scissor testing on/off
void scissorTest(bool testScissor);
void scissorArea(int left, int bottom, int width, int height);

/// Turn face culling on/off
void culling(bool doCulling);
/// face=[GL_FRONT, GL_BACK, GL_FRONT_AND_BACK], initial: GL_BACK
void cullFace(unsigned int face);

/// Set polygon drawing mode
/// mode=[GL_POINT, GL_LINE, GL_FILL]
/// GL_FRONT and GL_BACK is deprecated in 3.2 core profile
void polygonMode(unsigned int mode);

/// Turn color mask RGBA components on/off
void colorMask(bool r, bool g, bool b, bool a);

/// Turn color mask on/off (all RGBA components)
void colorMask(bool b);

/// Set diameter, in pixels, of points
void pointSize(float size);

/// Set width, in pixels, of lines.
/// Value exceeding 1 will give GL_INVALID_VALUE depending on version
void lineWidth(float size);

// clears the default color buffer(buffer 0) with the provided color
void clearColor(float r, float g, float b, float a = 1.f);

// clears the depth buffer with the provided depth value
void clearDepth(float d = 1.f);

// clears the specified color buffer with the provided color
void clearBuffer(int buffer, float r, float g, float b, float a = 1.f);
}  // namespace gl
}  // namespace al

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

#endif  // include guard
