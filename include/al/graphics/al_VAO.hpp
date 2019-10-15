#ifndef INCLUDE_AL_VAO_HPP
#define INCLUDE_AL_VAO_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
 */

#include "al/graphics/al_BufferObject.hpp"
#include "al/graphics/al_OpenGL.hpp"

#include <cstddef>

namespace al {
/**
@brief VAO class
@ingroup Graphics
*/

class VAO : public GPUObject {
 public:
  enum DataType : unsigned int /* GLenum */ {
    BYTE = GL_BYTE,             /**< */
    UBYTE = GL_UNSIGNED_BYTE,   /**< */
    SHORT = GL_SHORT,           /**< */
    USHORT = GL_UNSIGNED_SHORT, /**< */
    INT = GL_INT,               /**< */
    UINT = GL_UNSIGNED_INT,     /**< */
    FLOAT = GL_FLOAT,           /**< */
    DOUBLE = GL_DOUBLE          /**< */
  };

  VAO();
  virtual ~VAO();

  virtual void onCreate() override;
  virtual void onDestroy() override;

  void bind();
  void unbind();

  void enableAttrib(unsigned int index);
  void disableAttrib(unsigned int index);
  void attribPointer(unsigned int index, BufferObject& buffer,
                     int size,  // components
                     unsigned int type = GL_FLOAT,
                     unsigned char normalized = GL_FALSE, int stride = 0,
                     void const* offset = NULL  // offset of the first component
  );
};

}  // namespace al

#endif