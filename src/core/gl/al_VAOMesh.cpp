#include "al/core/gl/al_VAOMesh.hpp"
#include <iostream>

using namespace al;

std::unordered_map<unsigned int, unsigned int> VAOMesh::mPrimMap = {
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

void VAOMesh::update() {
  mGLPrimMode = mPrimMap[mPrimitive];
  vao.validate();
  vao.bind();
  updateAttrib(vertices(), position_att);
  updateAttrib(colors(), color_att);
  updateAttrib(texCoord2s(), texcoord2d_att);
  updateAttrib(normals(), normal_att);
  updateAttrib(texCoord3s(), texcoord3d_att);
  updateAttrib(texCoord1s(), texcoord1d_att);
  vao.unbind();
  if (indices().size() > 0) {
    if (!index_buffer.created()) {
      index_buffer.create();
      index_buffer.bufferType(GL_ELEMENT_ARRAY_BUFFER);
    }
    index_buffer.bind();
    index_buffer.data(
      sizeof(unsigned int) * indices().size(),
      indices().data()
    );
    index_buffer.unbind();
  }
}

template <typename T>
void VAOMesh::updateAttrib(
  std::vector<T> const& data, MeshAttrib& att
) {
  // only enable attribs with content
  if (data.size() > 0) {
    vao.enableAttrib(att.index);
  }
  else {
    vao.disableAttrib(att.index);
    return;
  }

  // buffer yet created, make it and set vao to point to it
  if (!att.buffer.created()) {
    att.buffer.create();
    vao.attribPointer(att.index, att.buffer, att.size);
  }

  // upload CPU size data to buffer in GPU
  auto s = sizeof(T);
  att.buffer.bind();
  att.buffer.data(s * data.size(), data.data());
  att.buffer.unbind(); 
}

template void VAOMesh::updateAttrib<float>(
  std::vector<float> const& data, MeshAttrib& att
);

template void VAOMesh::updateAttrib<Vec2f>(
  std::vector<Vec2f> const& data, MeshAttrib& att
);

template void VAOMesh::updateAttrib<Vec3f>(
  std::vector<Vec3f> const& data, MeshAttrib& att
);

template void VAOMesh::updateAttrib<Vec4f>(
  std::vector<Vec4f> const& data, MeshAttrib& att
);

void VAOMesh::draw() {
  vao.bind();
  if (indices().size() > 0) {
    index_buffer.bind();
    glDrawElements(mGLPrimMode, indices().size(), GL_UNSIGNED_INT, NULL);
    index_buffer.unbind();
  }
  else {
    glDrawArrays(mGLPrimMode, 0, vertices().size());
  }
  vao.unbind();
}