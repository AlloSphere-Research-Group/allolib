#ifndef INCLUDE_AL_OPENGL_HPP
#define INCLUDE_AL_OPENGL_HPP

#include "al/core/gl/al_GLEW.hpp"

namespace al {


enum AntiAliasMode {
	DONT_CARE				= GL_DONT_CARE,				/**< No preference */
	FASTEST					= GL_FASTEST,				/**< Fastest render, possibly lower quality */
	NICEST					= GL_NICEST					/**< Highest quality, possibly slower render */
};


enum AttributeBit {
	COLOR_BUFFER_BIT		= GL_COLOR_BUFFER_BIT,		/**< Color-buffer bit */
	DEPTH_BUFFER_BIT		= GL_DEPTH_BUFFER_BIT,		/**< Depth-buffer bit */
	// ENABLE_BIT				= GL_ENABLE_BIT,			/**< Enable bit */
	// VIEWPORT_BIT			= GL_VIEWPORT_BIT			/**< Viewport bit */
};

enum BlendFunc {
	SRC_ALPHA				= GL_SRC_ALPHA,				/**< */
	ONE_MINUS_SRC_ALPHA		= GL_ONE_MINUS_SRC_ALPHA,	/**< */
	SRC_COLOR				= GL_SRC_COLOR,				/**< */
	ONE_MINUS_SRC_COLOR		= GL_ONE_MINUS_SRC_COLOR,	/**< */
	DST_ALPHA				= GL_DST_ALPHA,				/**< */
	ONE_MINUS_DST_ALPHA		= GL_ONE_MINUS_DST_ALPHA,	/**< */
	DST_COLOR				= GL_DST_COLOR,				/**< */
	ONE_MINUS_DST_COLOR		= GL_ONE_MINUS_DST_COLOR,	/**< */
	ZERO					= GL_ZERO,					/**< */
	ONE						= GL_ONE,					/**< */
	SRC_ALPHA_SATURATE		= GL_SRC_ALPHA_SATURATE		/**< */
};

enum BlendEq {
	FUNC_ADD				= GL_FUNC_ADD,				/**< Source + destination */
	FUNC_SUBTRACT			= GL_FUNC_SUBTRACT,			/**< Source - destination */
	FUNC_REVERSE_SUBTRACT	= GL_FUNC_REVERSE_SUBTRACT, /**< Destination - source */
	MIN						= GL_MIN,					/**< Minimum value of source and destination */
	MAX						= GL_MAX					/**< Maximum value of source and destination */
};

enum Capability {
	BLEND					= GL_BLEND,					/**< Blend rather than replace existing colors with new colors */
	DEPTH_TEST				= GL_DEPTH_TEST,			/**< Test depth of incoming fragments */
	SCISSOR_TEST			= GL_SCISSOR_TEST,			/**< Crop fragments according to scissor region */
	CULL_FACE				= GL_CULL_FACE,				/**< Cull faces */
};

enum DataType {
	BYTE					= GL_BYTE,					/**< */
	UBYTE					= GL_UNSIGNED_BYTE,			/**< */
	SHORT					= GL_SHORT,					/**< */
	USHORT					= GL_UNSIGNED_SHORT,		/**< */
	INT						= GL_INT,					/**< */
	UINT					= GL_UNSIGNED_INT,			/**< */
	BYTES_2					= GL_2_BYTES,				/**< */
	BYTES_3					= GL_3_BYTES,				/**< */
	BYTES_4					= GL_4_BYTES,				/**< */
	FLOAT					= GL_FLOAT,					/**< */
	DOUBLE					= GL_DOUBLE					/**< */
};

enum Face {
	FRONT					= GL_FRONT,					/**< Front face */
	BACK					= GL_BACK,					/**< Back face */
	FRONT_AND_BACK			= GL_FRONT_AND_BACK			/**< Front and back face */
};

enum Format {
	DEPTH_COMPONENT			= GL_DEPTH_COMPONENT,		/**< */
	LUMINANCE				= GL_LUMINANCE,				/**< */
	LUMINANCE_ALPHA			= GL_LUMINANCE_ALPHA,		/**< */
	RED						= GL_RED,					/**< */
	GREEN					= GL_GREEN,					/**< */
	BLUE					= GL_BLUE,					/**< */
	ALPHA					= GL_ALPHA,					/**< */
	RGB						= GL_RGB,					/**< */
	BGR						= GL_BGR,					/**< */
	RGBA					= GL_RGBA,					/**< */
	BGRA					= GL_BGRA					/**< */
};

enum PolygonMode {
	POINT					= GL_POINT,					/**< Render only points at each vertex */
	LINE					= GL_LINE,					/**< Render only lines along vertex path */
	FILL					= GL_FILL					/**< Render vertices normally according to primitive */
};

enum Primitive {
	POINTS					= GL_POINTS,				/**< Points */
	LINES					= GL_LINES,					/**< Connect sequential vertex pairs with lines */
	LINE_STRIP				= GL_LINE_STRIP,			/**< Connect sequential vertices with a continuous line */
	LINE_LOOP				= GL_LINE_LOOP,				/**< Connect sequential vertices with a continuous line loop */
	TRIANGLES				= GL_TRIANGLES,				/**< Draw triangles using sequential vertex triplets */
	TRIANGLE_STRIP			= GL_TRIANGLE_STRIP,		/**< Draw triangle strip using sequential vertices */
	TRIANGLE_FAN			= GL_TRIANGLE_FAN,			/**< Draw triangle fan using sequential vertices */
	QUADS					= GL_QUADS,					/**< Draw quadrilaterals using sequential vertex quadruplets */
	QUAD_STRIP				= GL_QUAD_STRIP,			/**< Draw quadrilateral strip using sequential vertices */
	POLYGON					= GL_POLYGON				/**< Draw polygon using sequential vertices */
};

enum Target {
	TEXTURE_1D				= GL_TEXTURE_1D,
	TEXTURE_2D				= GL_TEXTURE_2D,
	TEXTURE_3D				= GL_TEXTURE_3D,
	NO_TARGET				= 0
};

enum Wrap {
	CLAMP					= GL_CLAMP,
	CLAMP_TO_BORDER			= GL_CLAMP_TO_BORDER,
	CLAMP_TO_EDGE			= GL_CLAMP_TO_EDGE,
	MIRRORED_REPEAT			= GL_MIRRORED_REPEAT,
	REPEAT					= GL_REPEAT
};

enum Filter {
	NEAREST					= GL_NEAREST,
	LINEAR					= GL_LINEAR,
	// first term is within mipmap level, second term is between mipmap levels:
	NEAREST_MIPMAP_NEAREST	= GL_NEAREST_MIPMAP_NEAREST,
	LINEAR_MIPMAP_NEAREST	= GL_LINEAR_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR	= GL_NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_LINEAR	= GL_LINEAR_MIPMAP_LINEAR,
};

/// Get current GPU error string
/// \returns the error string or an empty string if no error
///
inline const char * errorString(bool verbose=false) {
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

/// Returns number of components for given color type
inline int numComponents(Format v) {
	switch(v){
		case RGBA:
		case BGRA:				return 4;
		case RGB:
		case BGR:				return 3;
		case LUMINANCE_ALPHA:	return 2;
		case DEPTH_COMPONENT:
		case LUMINANCE:
		case RED:
		case GREEN:
		case BLUE:
		case ALPHA:				return 1;
		default:				return 0;
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
template<> DataType toDataType<char>(){ return BYTE; }
template<> DataType toDataType<unsigned char>(){ return UBYTE; }
template<> DataType toDataType<short>(){ return SHORT; }
template<> DataType toDataType<unsigned short>(){ return USHORT; }
template<> DataType toDataType<int>(){ return INT; }
template<> DataType toDataType<unsigned int>(){ return UINT; }
template<> DataType toDataType<float>(){ return FLOAT; }
template<> DataType toDataType<double>(){ return DOUBLE; }

}

#endif