#include "al/core/gl/al_BufferObject.hpp"

namespace al{

BufferObject::BufferObject():
  mType(GL_ARRAY_BUFFER),
  mUsage(GL_DYNAMIC_DRAW),
  // mDataType(GL_FLOAT),
  mMapMode(GL_MAP_READ_BIT | GL_MAP_WRITE_BIT),
  mNumComps(0), mNumElems(0)
{
  //
}

BufferObject::~BufferObject(){
  destroy();
}

int BufferObject::size() const {
  return mNumElems * mNumComps;// * size_in_bytes(mDataType);
}

// unsigned int BufferObject::dataType() const { return mDataType; }
int BufferObject::numComps() const { return mNumComps; }
int BufferObject::numElems() const { return mNumElems; }

void BufferObject::bufferType(unsigned int v){ mType=v; }
void BufferObject::usage(unsigned int v){ mUsage=v; }

void BufferObject::bind(){
  glBindBuffer(mType, id());
}

void BufferObject::unbind() const {
  glBindBuffer(mType, 0);
}

// #ifdef AL_GRAPHICS_USE_OPENGL
void BufferObject::mapMode(unsigned int v){ mMapMode=v; }

float* BufferObject::map(){
  return (float*)glMapBuffer(mType, mMapMode);
}

bool BufferObject::unmap(){
  return glUnmapBuffer(mType)==GL_TRUE;
}
// #endif

void BufferObject::onCreate() {
  glGenBuffers(1, &mID);
}

void BufferObject::onDestroy() {
  glDeleteBuffers(1, &mID);
}

void BufferObject::data(int numComps, int numElems, float const* src) {
  glBufferData(
    mType,
    sizeof(float) * numComps * numElems,
    src,
    mUsage
  );
}

// void BufferObject::print() const {
  // printf("%s: %s %s (%d comps %d elems [%d bytes])\n", toString(mType), toString(mUsage), toString(mDataType), mNumComps, mNumElems, size());
// }

} // al::
