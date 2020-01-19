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

/// Turn blending on/off
void blending(bool doBlend);

/// Turn the depth mask on/off
void depthMask(bool maskDepth);

/// Turn depth testing on/off
void depthTesting(bool testDepth);
void viewport(int left, int bottom, int width, int height);

/// Turn scissor testing on/off
void scissorTest(bool testScissor);
void scissorArea(int left, int bottom, int width, int height);

/// Turn face culling on/off
void faceCulling(bool doCulling);
/// face=[GL_FRONT, GL_BACK, GL_FRONT_AND_BACK], initial: GL_BACK
void faceToCull(unsigned int face);

/// Set diameter, in pixels, of points
void pointSize(float size);

/// Set width, in pixels, of lines.
/// Value exceeding 1 will give GL_INVALID_VALUE depending on version
void lineWidth(float size);

/// Set polygon drawing mode
/// mode=[GL_POINT, GL_LINE, GL_FILL]
/// GL_FRONT and GL_BACK is deprecated in 3.2 core profile
void polygonMode(unsigned int mode);

/// Set blend mode
/// src,dst=[GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR,
///          GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA,
///          GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
///          GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR,
///          GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA]
/// eq=[GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT,
///     GL_MIN, GL_MAX]
void blendMode(unsigned int src, unsigned int dst, unsigned int eq);

/// Turn color mask RGBA components on/off
void colorMask(bool r, bool g, bool b, bool a);

/// Turn color mask on/off (all RGBA components)
void colorMask(bool b);

void clearColor(float r = 0, float g = 0, float b = 0, float a = 1);
void clearDepth(float d = 1);
void clearBuffer(int buffer, float r = 0, float g = 0, float b = 0,
                 float a = 1);
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

/// Draw only points of vertices
inline void polygonPoint() { polygonMode(GL_POINT); }
/// Draw only edges of polygons with lines
inline void polygonLine() { polygonMode(GL_LINE); }
/// Draw filled polygons
inline void polygonFill() { polygonMode(GL_FILL); }

inline void blendDisable() { blending(false); }

/// Set blend mode to additive (symmetric additive lighten)
inline void blendAdd() {
  blending(true);
  blendMode(GL_SRC_ALPHA, GL_ONE, GL_FUNC_ADD);
}

/// Set blend mode to subtractive (symmetric additive darken)
inline void blendSub() {
  blending(true);
  blendMode(GL_SRC_ALPHA, GL_ONE, GL_FUNC_REVERSE_SUBTRACT);
}

/// Set blend mode to screen (symmetric multiplicative lighten)
inline void blendScreen() {
  blending(true);
  blendMode(GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_FUNC_ADD);
}

/// Set blend mode to multiplicative (symmetric multiplicative darken)
inline void blendMult() {
  blending(true);
  blendMode(GL_DST_COLOR, GL_ZERO, GL_FUNC_ADD);
}

/// Set blend mode to transparent (asymmetric)
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
