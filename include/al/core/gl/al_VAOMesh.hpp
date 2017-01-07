#pragma once

#include "al/core/gl/al_BufferObject.hpp"
#include "al/core/gl/al_Mesh.hpp"
#include "al/core/gl/al_VAO.hpp"

#include <unordered_map>

namespace al {

class VAOMesh : public Mesh {
public:
    static std::unordered_map<Primitive, unsigned int> mPrimMap;

    unsigned int mGLPrimMode = GL_TRIANGLES;
    VAO vao;
    BufferObject
        position_buffer,
        color_buffer,
        // texcoord_buffer,
        normal_buffer
        // index_buffer
    ;

    void init();
    void update();

    void bind();
    void unbind();

    template <typename T>
    void updateAttrib(
        std::vector<T> const& data,
        BufferObject& buffer,
        unsigned int index
    );

    void draw();
};

}