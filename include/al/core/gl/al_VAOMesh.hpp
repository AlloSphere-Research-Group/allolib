#ifndef INCLUDE_AL_VAOMESH_HPP
#define INCLUDE_AL_VAOMESH_HPP

#include "al/core/gl/al_BufferObject.hpp"
#include "al/core/gl/al_Mesh.hpp"
#include "al/core/gl/al_VAO.hpp"

#include <unordered_map>

namespace al {

// layout (attribute location) for (GL) <-> (glsl shader)
enum AttribLayout: unsigned int {
  ATTRIB_POSITION = 0,
  ATTRIB_COLOR = 1,
  ATTRIB_TEXCOORD_2D = 2,
  ATTRIB_NORMAL = 3,
  ATTRIB_TEXCOORD_3D = 4, // also could be used for other purposes
  ATTRIB_TEXCOORD_1D = 5, //
};

class VAOMesh : public Mesh {
public:
    static std::unordered_map<unsigned int, unsigned int> mPrimMap;

    struct MeshAttrib {
        unsigned int index;
        int size;
        BufferObject buffer;
        MeshAttrib(unsigned int i, int s): index(i), size(s) {}
    };

    unsigned int mGLPrimMode = GL_TRIANGLES;
    VAO mVao;
    MeshAttrib
        mPositionAtt {ATTRIB_POSITION, 3},
        mColorAtt {ATTRIB_COLOR, 4},
        mTexcoord2dAtt {ATTRIB_TEXCOORD_2D, 2},
        mNormalAtt {ATTRIB_NORMAL, 3},
        mTexcoord3dAtt {ATTRIB_TEXCOORD_3D, 3},
        mTexcoord1dAtt {ATTRIB_TEXCOORD_1D, 1}
    ;
    BufferObject mIndexBuffer;

    void update();

    void bind();
    void unbind();

    template <typename T>
    void updateAttrib(
        std::vector<T> const& data, MeshAttrib& att
    );

    void draw();
};

}

#endif