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

void VAOMesh::update() {
  mGLPrimMode = mPrimMap[mPrimitive];
  vao.validate();
  vao.bind();
  updateAttrib(vertices(), position_att);
  updateAttrib(colors(), color_att);
  // updateAttrib(texCoord2s(), texcoord_att, 2);
  updateAttrib(normals(), normal_att);
  // updateAttrib(normals(), normal_att);
  // updateAttrib(indices(), index_buffer);
  vao.unbind();
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
  bind();
  glDrawArrays(mGLPrimMode, 0, vertices().size());
  unbind();
}