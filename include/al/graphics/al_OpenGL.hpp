#ifndef INCLUDE_AL_OPENGL_HPP
#define INCLUDE_AL_OPENGL_HPP

// #include "GL/glew.h"
#include "glad/glad.h"

namespace al {

namespace gl {

bool load();
bool loaded();

const char* versionString();

/// Get current GPU error string
/// \returns the error string or an empty string if no error
///
const char* errorString(bool verbose = false);

/// Print current GPU error state

/// @param[in] msg    Custom error message
/// @param[in] ID   Graphics object ID (-1 for none)
/// \returns whether there was an error
bool error(const char* msg = "", int ID = -1);

/// Returns number of bytes for given data type
int numBytes(GLenum v);

/// Get DataType associated with a basic C type
template <typename Type>
GLenum toDataType();

void blending(bool doBlend);
void depthMask(bool maskDepth);
void depthTest(bool testDepth);
void viewport(int left, int bottom, int width, int height);
void scissorTest(bool testScissor);
void scissorArea(int left, int bottom, int width, int height);
void faceCulling(bool doCulling);
/// face=[GL_FRONT, GL_BACK, GL_FRONT_AND_BACK], initial: GL_BACK
void faceToCull(unsigned int face);
void pointSize(float size);
/// mode=[GL_POINT, GL_LINE, GL_FILL]
void polygonMode(unsigned int mode);
/// src,dst=[GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR,
///          GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA,
///          GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
///          GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR,
///          GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA]
/// eq=[GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT,
///     GL_MIN, GL_MAX]
void blendMode(unsigned int src, unsigned int dst, unsigned int eq);
void clearColor(float r, float g, float b, float a);
void clearDepth(float d);
void clearBuffer(int buffer, float r, float g, float b, float a);
/// buffer=[GL_NONE, GL_FRONT_LEFT, GL_FRONT_RIGHT, GL_BACK_LEFT,
///         GL_BACK_RIGHT, GL_FRONT, GL_BACK, GL_LEFT, GL_RIGHT,
///         GL_FRONT_AND_BACK]
void bufferToDraw(unsigned int buffer);

/// inlined convenience functions
inline void cullFaceFront() {
  faceCulling(true);
  faceToCull(GL_FRONT);
}
inline void cullFaceBack() {
  faceCulling(true);
  faceToCull(GL_BACK);
}
inline void cullFaceBoth() {
  faceCulling(true);
  faceToCull(GL_FRONT_AND_BACK);
}
inline void cullFaceNone() { faceCulling(false); }

inline void ploygonPoint() { polygonMode(GL_POINT); }
inline void ploygonLine() { polygonMode(GL_LINE); }
inline void ploygonFill() { polygonMode(GL_FILL); }

inline void blendDisable() { blending(false); }
inline void blendAdd() {
  blending(true);
  blendMode(GL_SRC_ALPHA, GL_ONE, GL_FUNC_ADD);
}
inline void blendSub() {
  blending(true);
  blendMode(GL_SRC_ALPHA, GL_ONE, GL_FUNC_REVERSE_SUBTRACT);
}
inline void blendScreen() {
  blending(true);
  blendMode(GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_FUNC_ADD);
}
inline void blendMult() {
  blending(true);
  blendMode(GL_DST_COLOR, GL_ZERO, GL_FUNC_ADD);
}
inline void blendTrans() {
  blending(true);
  blendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD);
}

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
