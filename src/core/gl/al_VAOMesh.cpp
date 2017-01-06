#include "al/core/gl/al_VAOMesh.hpp"

using namespace al;

std::unordered_map<Primitive, unsigned int> VAOMesh::mPrimMap = {
    { POINTS, GL_POINTS },
    { LINES, GL_LINES },
    { LINE_STRIP, GL_LINE_STRIP },
    { LINE_LOOP, GL_LINE_LOOP },
    { TRIANGLES, GL_TRIANGLES },
    { TRIANGLE_STRIP, GL_TRIANGLE_STRIP },
    { TRIANGLE_FAN, GL_TRIANGLE_FAN },
    { LINES_ADJACENCY, GL_LINES_ADJACENCY },
    { LINE_STRIP_ADJACENCY, GL_LINE_STRIP_ADJACENCY },
    { TRIANGLES_ADJACENCY, GL_TRIANGLES_ADJACENCY },
    { TRIANGLE_STRIP_ADJACENCY, GL_TRIANGLE_STRIP_ADJACENCY }
};

void VAOMesh::bind() {
    vao.bind();
}

void VAOMesh::unbind() {
    vao.unbind();
}

void VAOMesh::init() {
    vao.create();
    position_buffer.create();
    color_buffer.create();
    // texcoord_buffer.create();
    // normal_buffer.create();
    // index_buffer.create();
    // index_buffer.bufferType(GL_ELEMENT_ARRAY_BUFFER);
    vao.bind();
    vao.enableAttrib(0);
    vao.attribPointer(0, position_buffer, 4);
    vao.enableAttrib(1);
    vao.attribPointer(1, color_buffer, 4);
    vao.unbind();
}

void VAOMesh::update() {
    mGLPrimMode = mPrimMap[mPrimitive];
    bind();
    updateAttrib(vertices(), position_buffer);
    updateAttrib(colors(), color_buffer);
    // updateAttrib(texCoord2s(), texcoord_buffer);
    // updateAttrib(normals(), normal_buffer);
    // updateAttrib(indices(), index_buffer);
    // vao.attribPointer(0, position_buffer, 4);
    // vao.attribPointer(1, color_buffer, 4);
    unbind();
}

template <typename T>
void VAOMesh::updateAttrib(std::vector<T> const& data, BufferObject& buffer) {
    if (data.size() == 0) {
        return;
    }
    auto s = sizeof(T);
    // if size changed, (re)allocate buffer
    if (s * data.size() != buffer.size()) {
        buffer.bind();
        buffer.data(
            s * data.size(),
            data.data()
        );
        buffer.unbind();
    }
    else { // just re-submit data
        buffer.bind();
        buffer.subdata(
            0,
            s * data.size(),
            data.data()
        );
        buffer.unbind(); 
    }
}

template void VAOMesh::updateAttrib<float>(
    std::vector<float> const& data, BufferObject& buffer
);

template void VAOMesh::updateAttrib<Vec2f>(
    std::vector<Vec2f> const& data, BufferObject& buffer
);

template void VAOMesh::updateAttrib<Vec3f>(
    std::vector<Vec3f> const& data, BufferObject& buffer
);

template void VAOMesh::updateAttrib<Vec4f>(
    std::vector<Vec4f> const& data, BufferObject& buffer
);

void VAOMesh::draw() {
    bind();
    glDrawArrays(mGLPrimMode, 0, vertices().size());
    unbind();
}