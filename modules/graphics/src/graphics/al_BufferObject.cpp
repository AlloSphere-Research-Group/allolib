#include "al/graphics/al_BufferObject.hpp"
#include "al/graphics/al_OpenGL.hpp"

#include <iostream>

namespace al {

BufferObject::BufferObject()
    : mType(GL_ARRAY_BUFFER),
      mUsage(GL_DYNAMIC_DRAW),
      // mMapMode(GL_MAP_READ_BIT | GL_MAP_WRITE_BIT),
      mSize(0) {
  //
}

BufferObject::~BufferObject() { destroy(); }

int BufferObject::size() const { return static_cast<int>(mSize); }

void BufferObject::bufferType(unsigned int v) { mType = v; }

void BufferObject::usage(unsigned int v) { mUsage = v; }

void BufferObject::bind() { glBindBuffer(mType, id()); }

void BufferObject::unbind() const { glBindBuffer(mType, 0); }

// #ifdef AL_GRAPHICS_USE_OPENGL
// void BufferObject::mapMode(unsigned int v){
//   mMapMode=v;
// }

// void* BufferObject::map(){
//   return glMapBuffer(mType, mMapMode);
// }

// bool BufferObject::unmap(){
//   return glUnmapBuffer(mType)==GL_TRUE;
// }
// #endif

void BufferObject::onCreate() { glGenBuffers(1, &mID); }

void BufferObject::onDestroy() { glDeleteBuffers(1, &mID); }

void BufferObject::data(size_t size, void const* src) {
  glBufferData(mType, size, src, mUsage);
  mSize = size;
  // GLint s {0};
  // glGetBufferParameteriv(mType, GL_BUFFER_SIZE, &s);
  // if (s == size) {
  //   mSize = size;
  // }
  // else {
  //   std::cout << "buffer size does not match requested size" << std::endl;
  // }
}

void BufferObject::subdata(int offset, int size, void const* src) {
  glBufferSubData(mType, offset, size, src);
}

}  // namespace al
