#ifndef INCLUDE_AL_OPENGL_HPP
#define INCLUDE_AL_OPENGL_HPP

#include "al/core/graphics/al_GLEW.hpp"

namespace al {

enum DataType {
    BYTE                    = GL_BYTE,                  /**< */
    UBYTE                   = GL_UNSIGNED_BYTE,         /**< */
    SHORT                   = GL_SHORT,                 /**< */
    USHORT                  = GL_UNSIGNED_SHORT,        /**< */
    INT                     = GL_INT,                   /**< */
    UINT                    = GL_UNSIGNED_INT,          /**< */
    BYTES_2                 = GL_2_BYTES,               /**< */
    BYTES_3                 = GL_3_BYTES,               /**< */
    BYTES_4                 = GL_4_BYTES,               /**< */
    FLOAT                   = GL_FLOAT,                 /**< */
    DOUBLE                  = GL_DOUBLE                 /**< */
};

enum class Target : unsigned int {
    TEXTURE_1D                   = GL_TEXTURE_1D,
    TEXTURE_2D                   = GL_TEXTURE_2D,
    TEXTURE_3D                   = GL_TEXTURE_3D,
    TEXTURE_1D_ARRAY             = GL_TEXTURE_1D_ARRAY,
    TEXTURE_2D_ARRAY             = GL_TEXTURE_2D_ARRAY,
    TEXTURE_RECTANGLE            = GL_TEXTURE_RECTANGLE,
    TEXTURE_CUBE_MAP             = GL_TEXTURE_CUBE_MAP,
    TEXTURE_BUFFER               = GL_TEXTURE_BUFFER,
    TEXTURE_2D_MULTISAMPLE       = GL_TEXTURE_2D_MULTISAMPLE,
    TEXTURE_2D_MULTISAMPLE_ARRAY = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
    NO_TARGET               = 0
};

enum class Wrap : unsigned int {
    CLAMP_TO_EDGE           = GL_CLAMP_TO_EDGE,
    CLAMP_TO_BORDER         = GL_CLAMP_TO_BORDER,
    MIRRORED_REPEAT         = GL_MIRRORED_REPEAT,
    REPEAT                  = GL_REPEAT
};

enum class Filter : unsigned int {
    NEAREST                 = GL_NEAREST,
    LINEAR                  = GL_LINEAR,
    // first term is within mipmap level, second term is between mipmap levels:
    NEAREST_MIPMAP_NEAREST  = GL_NEAREST_MIPMAP_NEAREST,
    LINEAR_MIPMAP_NEAREST   = GL_LINEAR_MIPMAP_NEAREST,
    NEAREST_MIPMAP_LINEAR   = GL_NEAREST_MIPMAP_LINEAR,
    LINEAR_MIPMAP_LINEAR    = GL_LINEAR_MIPMAP_LINEAR
};

enum class Format : unsigned int {
    RED             = GL_RED,
    RG              = GL_RG,
    RGB             = GL_RGB,
    BGR             = GL_BGR,
    RGBA            = GL_RGBA,
    BGRA            = GL_BGRA,
    DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
    DEPTH_STENCIL   = GL_DEPTH_STENCIL
};

enum class Internal : unsigned int {
    RGBA32F = GL_RGBA32F,
    RGBA16 = GL_RGBA16,
    RGBA16F = GL_RGBA16F,
    RGBA8 = GL_RGBA8,
    SRGB8_ALPHA8 = GL_SRGB8_ALPHA8,

    RG32F = GL_RG32F,
    RG16 = GL_RG16,
    RG16F = GL_RG16F,
    RG8 = GL_RG8,

    R32F = GL_R32F,
    R16F = GL_R16F,
    R8 = GL_R8,

    RGB32F = GL_RGB32F,
    RGB16F = GL_RGB16F,
    RGB16 = GL_RGB16,
    RGB8 = GL_RGB8,
    SRGB8 = GL_SRGB8,

    DEPTH_COMPONENT32F = GL_DEPTH_COMPONENT32F,
    DEPTH_COMPONENT24 = GL_DEPTH_COMPONENT24,
    DEPTH_COMPONENT16 = GL_DEPTH_COMPONENT16,
    DEPTH32F_STENCIL8 = GL_DEPTH32F_STENCIL8,
    DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8

    // there's more...
};

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

}

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

/// Returns number of components for given color type
inline int numComponents(Format v) {
    switch(v){
        case Format::RED:               return 1;
        case Format::RG:                return 2;
        case Format::RGB:
        case Format::BGR:               return 3;
        case Format::RGBA:
        case Format::BGRA:              return 4;
        case Format::DEPTH_COMPONENT:   return 1;
        case Format::DEPTH_STENCIL:     return 2;
        default:                return 0;
    };
}

/// Returns number of bytes for given data type
inline int numBytes(DataType v) {
    #define CS(a,b) case a: return sizeof(b);
    switch(v){
        CS(BYTE, GLbyte)
        CS(UBYTE, GLubyte)
        CS(SHORT, GLshort)
        CS(USHORT, GLushort)
        CS(INT, GLint)
        CS(UINT, GLuint)
        CS(BYTES_2, char[2])
        CS(BYTES_3, char[3])
        CS(BYTES_4, char[4])
        CS(FLOAT, GLfloat)
        CS(DOUBLE, GLdouble)
        default: return 0;
    };
    #undef CS
}

/// Get DataType associated with a basic C type
template<typename Type>
inline DataType toDataType();
template<> inline DataType toDataType<char>(){ return BYTE; }
template<> inline DataType toDataType<unsigned char>(){ return UBYTE; }
template<> inline DataType toDataType<short>(){ return SHORT; }
template<> inline DataType toDataType<unsigned short>(){ return USHORT; }
template<> inline DataType toDataType<int>(){ return INT; }
template<> inline DataType toDataType<unsigned int>(){ return UINT; }
template<> inline DataType toDataType<float>(){ return FLOAT; }
template<> inline DataType toDataType<double>(){ return DOUBLE; }

}

#endif