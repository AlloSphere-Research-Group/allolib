#include "al/core/gl/al_VAOMesh.hpp"
#include <iostream>

using namespace al;

std::unordered_map<unsigned int, unsigned int> VAOMesh::mPrimMap = {
  { Mesh::POINTS, GL_POINTS },
  { Mesh::LINES, GL_LINES },
  { Mesh::LINE_STRIP, GL_LINE_STRIP },
  { Mesh::LINE_LOOP, GL_LINE_LOOP },
  { Mesh::TRIANGLES, GL_TRIANGLES },
  { Mesh::TRIANGLE_STRIP, GL_TRIANGLE_STRIP },
  { Mesh::TRIANGLE_FAN, GL_TRIANGLE_FAN },
  { Mesh::LINES_ADJACENCY, GL_LINES_ADJACENCY },
  { Mesh::LINE_STRIP_ADJACENCY, GL_LINE_STRIP_ADJACENCY },
  { Mesh::TRIANGLES_ADJACENCY, GL_TRIANGLES_ADJACENCY },
  { Mesh::TRIANGLE_STRIP_ADJACENCY, GL_TRIANGLE_STRIP_ADJACENCY }
};

void VAOMesh::bind() {
  mVao.bind();
}

void VAOMesh::unbind() {
  mVao.unbind();
}

void VAOMesh::update() {
  mGLPrimMode = mPrimMap[mPrimitive];
  mVao.validate();
  mVao.bind();
  updateAttrib(vertices(), mPositionAtt);
  updateAttrib(colors(), mColorAtt);
  updateAttrib(texCoord2s(), mTexcoord2dAtt);
  updateAttrib(normals(), mNormalAtt);
  updateAttrib(texCoord3s(), mTexcoord3dAtt);
  updateAttrib(texCoord1s(), mTexcoord1dAtt);
  mVao.unbind();
  if (indices().size() > 0) {
    if (!mIndexBuffer.created()) {
      mIndexBuffer.create();
      mIndexBuffer.bufferType(GL_ELEMENT_ARRAY_BUFFER);
    }
    mIndexBuffer.bind();
    mIndexBuffer.data(
      sizeof(unsigned int) * indices().size(),
      indices().data()
    );
    mIndexBuffer.unbind();
  }
}

template <typename T>
void VAOMesh::updateAttrib(
  std::vector<T> const& data, MeshAttrib& att
) {
  // only enable attribs with content
  if (data.size() > 0) {
    mVao.enableAttrib(att.index);
  }
  else {
    mVao.disableAttrib(att.index);
    return;
  }

  // buffer yet created, make it and set vao to point to it
  if (!att.buffer.created()) {
    att.buffer.create();
    mVao.attribPointer(att.index, att.buffer, att.size);
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
  mVao.bind();
  if (indices().size() > 0) {
    mIndexBuffer.bind();
    glDrawElements(mGLPrimMode, indices().size(), GL_UNSIGNED_INT, NULL);
    mIndexBuffer.unbind();
  }
  else {
    glDrawArrays(mGLPrimMode, 0, vertices().size());
  }
  mVao.unbind();
}