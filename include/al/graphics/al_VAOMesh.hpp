#ifndef INCLUDE_AL_VAOMESH_HPP
#define INCLUDE_AL_VAOMESH_HPP

/*    Keehong Youn, 2017, younkeehong@gmail.com
 */

#include "al/graphics/al_BufferObject.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_VAO.hpp"

#include <iostream>
#include <memory>
#include <unordered_map>

namespace al {

// layout (attribute location) for (GL) <-> (glsl shader)
enum AttribLayout : unsigned int {
  ATTRIB_POSITION = 0,
  ATTRIB_COLOR = 1,
  ATTRIB_TEXCOORD_2D = 2,
  ATTRIB_NORMAL = 3
  // ATTRIB_TEXCOORD_3D = 4, // also could be used for other purposes
  // ATTRIB_TEXCOORD_1D = 5, //
};

/**
@brief VAOMesh class
@ingroup Graphics
Stores gpu objects as shared pointer to that this class can be copied for moved
 this is useful for storing this class in std::vector, etc.
*/

class VAOMesh : public Mesh {
 public:
  struct MeshAttrib {
    unsigned int index;
    int size;
    BufferObject buffer;
    MeshAttrib(unsigned int i, int s) : index(i), size(s) {}
  };

  struct VAOWrapper {
    unsigned int GLPrimMode = GL_TRIANGLES;
    VAO vao;
    MeshAttrib positionAtt{ATTRIB_POSITION, 3}, colorAtt{ATTRIB_COLOR, 4},
        texcoord2dAtt{ATTRIB_TEXCOORD_2D, 2},
        normalAtt{ATTRIB_NORMAL, 3}  // mTexcoord3dAtt {ATTRIB_TEXCOORD_3D, 3},
                                     // mTexcoord1dAtt {ATTRIB_TEXCOORD_1D, 1}
    ;
    BufferObject indexBuffer;
  };

  std::shared_ptr<VAOWrapper> vaoWrapper;

  VAOMesh();
  VAOMesh(Primitive p);
  VAOMesh(VAOMesh const& other);
  VAOMesh(VAOMesh&& other);
  VAOMesh& operator=(VAOMesh const& other);
  VAOMesh& operator=(VAOMesh&& other);

  VAO& vao() { return vaoWrapper->vao; }
  MeshAttrib& positionAtt() { return vaoWrapper->positionAtt; }
  MeshAttrib& colorAtt() { return vaoWrapper->colorAtt; }
  MeshAttrib& texcoord2dAtt() { return vaoWrapper->texcoord2dAtt; }
  MeshAttrib& normalAtt() { return vaoWrapper->normalAtt; }
  BufferObject& indexBuffer() { return vaoWrapper->indexBuffer; }

  void update();

  void bind();
  void unbind();

  template <typename T>
  void updateAttrib(std::vector<T> const& data, MeshAttrib& att);

  void draw();
};

}  // namespace al

#endif