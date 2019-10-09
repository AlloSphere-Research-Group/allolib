#include "al/graphics/al_VAOMesh.hpp"
#include <iostream>

using namespace al;

VAOMesh::VAOMesh() { vaoWrapper = std::make_shared<VAOWrapper>(); }
VAOMesh::VAOMesh(Primitive p) : Mesh(p) {
  vaoWrapper = std::make_shared<VAOWrapper>();
}

// when copying, make new vao
VAOMesh::VAOMesh(VAOMesh const& other) : Mesh(other) {
  vaoWrapper = std::make_shared<VAOWrapper>();
  if (gl::loaded()) update();
  // std::cout << "copy ctor" << std::endl;
}

// when moving, move vao
VAOMesh::VAOMesh(VAOMesh&& other) : Mesh(other) {
  vaoWrapper = other.vaoWrapper;
  // std::cout << "move ctor" << std::endl;
}

// when copying, make new vao
VAOMesh& VAOMesh::operator=(VAOMesh const& other) {
  copy(other);
  vaoWrapper = std::make_shared<VAOWrapper>();
  if (gl::loaded()) update();
  // std::cout << "copy assignment" << std::endl;
  return *this;
}

// when moving, move vao
VAOMesh& VAOMesh::operator=(VAOMesh&& other) {
  copy(other);
  vaoWrapper = other.vaoWrapper;
  // std::cout << "move assignment" << std::endl;
  return *this;
}

void VAOMesh::bind() { vao().bind(); }

void VAOMesh::unbind() { vao().unbind(); }

void VAOMesh::update() {
  vaoWrapper->GLPrimMode = mPrimitive;
  vao().validate();
  vao().bind();
  updateAttrib(vertices(), positionAtt());
  updateAttrib(colors(), colorAtt());
  updateAttrib(texCoord2s(), texcoord2dAtt());
  updateAttrib(normals(), normalAtt());
  // updateAttrib(texCoord3s(), mTexcoord3dAtt);
  // updateAttrib(texCoord1s(), mTexcoord1dAtt);
  // vao().unbind();
  if (indices().size() > 0) {
    if (!indexBuffer().created()) {
      // mIndexBuffer.create();
      // mIndexBuffer.bufferType(GL_ELEMENT_ARRAY_BUFFER);
      indexBuffer().create();
      indexBuffer().bufferType(GL_ELEMENT_ARRAY_BUFFER);
    }
    indexBuffer().bind();
    indexBuffer().data(sizeof(unsigned int) * indices().size(),
                       indices().data());
    // indexBuffer().unbind();
  }
}

template <typename T>
void VAOMesh::updateAttrib(std::vector<T> const& data, MeshAttrib& att) {
  // only enable attribs with content
  if (data.size() > 0) {
    vao().enableAttrib(att.index);
  } else {
    vao().disableAttrib(att.index);
    return;
  }

  // buffer yet created, make it and set vao to point to it
  if (!att.buffer.created()) {
    att.buffer.create();
    vao().attribPointer(att.index, att.buffer, att.size);
  }

  // upload CPU size data to buffer in GPU
  auto s = sizeof(T);
  att.buffer.bind();
  att.buffer.data(s * data.size(), data.data());
  // att.buffer.unbind();
}

template void VAOMesh::updateAttrib<float>(std::vector<float> const& data,
                                           MeshAttrib& att);

template void VAOMesh::updateAttrib<Vec2f>(std::vector<Vec2f> const& data,
                                           MeshAttrib& att);

template void VAOMesh::updateAttrib<Vec3f>(std::vector<Vec3f> const& data,
                                           MeshAttrib& att);

template void VAOMesh::updateAttrib<Vec4f>(std::vector<Vec4f> const& data,
                                           MeshAttrib& att);

void VAOMesh::draw() {
  vao().bind();
  if (indices().size() > 0) {
    indexBuffer().bind();
    int num_indices = (int)indices().size();
    glDrawElements(vaoWrapper->GLPrimMode, num_indices, GL_UNSIGNED_INT, NULL);
    // indexBuffer().unbind();
  } else {
    int num_vertices = (int)vertices().size();
    glDrawArrays(vaoWrapper->GLPrimMode, 0, num_vertices);
  }
  // vao().unbind();
}