#ifndef INCLUDE_AL_EASYVAO_HPP
#define INCLUDE_AL_EASYVAO_HPP

/*  VAO wrapper ready to receive mesh data

    - wraps a vao for render call and vertex buffer handles for attributes
    - This class does not store cpu side data
    - Any user data format could be used
      as long as they are stored as contiguous memory

    Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/graphics/al_BufferObject.hpp"
#include "al/graphics/al_VAO.hpp"

namespace al {

/**
@brief EasyVAO class
@ingroup Graphics
*/
struct EasyVAO : VAO {
  // attribute locations specified in default shader
  // layout (location = n) in vertex shader
  enum AttribLayout : unsigned int {
    LAYOUT_POSITION = 0,
    LAYOUT_COLOR = 1,
    LAYOUT_TEXCOORD = 2,
    LAYOUT_NORMAL = 3,
  };

  // attribute type specified in default shader
  // 3 -> vec3, etc ...
  enum AttribDimension : unsigned int {
    DIMENSION_POSITION = 3,
    DIMENSION_COLOR = 4,
    DIMENSION_TEXCOORD = 2,
    DIMENSION_NORMAL = 3,
  };

  // buffer to hold attribute data (position, color, texcoord, ...)
  struct MeshAttrib {
    unsigned int layoutIndex;    // attrib layout?
    unsigned int dataDimension;  // vec3? vec4? ...
    BufferObject buffer;
    MeshAttrib(unsigned int i, unsigned int d)
        : layoutIndex(i), dataDimension(d) {}
  };

  void update(void* data, size_t typeSize, size_t arraySize, MeshAttrib& attrib,
              unsigned int dataType = GL_FLOAT);

  template <typename T>
  void updatePosition(T* data, int arraySize,
                      unsigned int dataType = GL_FLOAT) {
    mNumVertices = static_cast<int>(arraySize);
    update(data, sizeof(T), arraySize, mPositionAtt, dataType);
  }

  template <typename T>
  void updateColor(T* data, size_t arraySize,
                   unsigned int dataType = GL_FLOAT) {
    update(data, sizeof(T), arraySize, mColorAtt, dataType);
  }

  template <typename T>
  void updateTexcoord(T* data, size_t arraySize,
                      unsigned int dataType = GL_FLOAT) {
    update(data, sizeof(T), arraySize, mTexcoord2dAtt, dataType);
  }

  template <typename T>
  void updateNormal(T* data, size_t arraySize,
                    unsigned int dataType = GL_FLOAT) {
    update(data, sizeof(T), arraySize, mNormalAtt, dataType);
  }

  // indices must be unsigned int
  void updateIndices(const unsigned int* data, size_t size);

  // GL_TRIANGLES, GL_TRIANGLE_STRIP, ...
  void primitive(unsigned int prim);

  void draw();

  void updateWithoutBinding(const void* data, size_t typeSize, size_t arraySize,
                            MeshAttrib& attrib,
                            unsigned int dataType = GL_FLOAT);

  unsigned int mGLPrimMode = GL_TRIANGLES;
  int mNumVertices = 0;
  int mNumIndices = 0;
  MeshAttrib mPositionAtt{LAYOUT_POSITION, DIMENSION_POSITION},
      mColorAtt{LAYOUT_COLOR, DIMENSION_COLOR},
      mTexcoord2dAtt{LAYOUT_TEXCOORD, DIMENSION_TEXCOORD},
      mNormalAtt{LAYOUT_NORMAL, DIMENSION_NORMAL};

  BufferObject mIndexBuffer;
};

}  // namespace al

#endif
