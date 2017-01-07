#pragma once

#include "al/core/gl/al_BufferObject.hpp"
#include "al/core/gl/al_Mesh.hpp"
#include "al/core/gl/al_VAO.hpp"

#include <unordered_map>

namespace al {

// layout for (GL) <-> (glsl shader)
enum AttribLayout: unsigned int {
  ATTRIB_POSITION = 0,
  ATTRIB_COLOR = 1,
  ATTRIB_TEXCOORD = 2,
  ATTRIB_NORMAL = 3,
};

class VAOMesh : public Mesh {
public:
    static std::unordered_map<Primitive, unsigned int> mPrimMap;

    struct MeshAttrib {
        unsigned int index;
        int size;
        BufferObject buffer;
        MeshAttrib(unsigned int i, int s): index(i), size(s) {}
    };

    unsigned int mGLPrimMode = GL_TRIANGLES;
    VAO vao;
    MeshAttrib
        position_att {ATTRIB_POSITION, 4},
        color_att {ATTRIB_COLOR, 4},
        // texcoord_att {ATTRIB_TEXCOORD, 2},
        normal_att {ATTRIB_NORMAL, 3}
        // index_att
    ;

    void init();
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