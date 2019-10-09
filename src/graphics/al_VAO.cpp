#include "al/graphics/al_VAO.hpp"

#include <iostream>

using namespace al;

VAO::VAO(){

};

VAO::~VAO() { destroy(); };

void VAO::onCreate() { glGenVertexArrays(1, &mID); }

void VAO::onDestroy() { glDeleteVertexArrays(1, &mID); }

void VAO::bind() { glBindVertexArray(mID); }

void VAO::unbind() { glBindVertexArray(0); }

void VAO::enableAttrib(unsigned int index) { glEnableVertexAttribArray(index); }

void VAO::disableAttrib(unsigned int index) {
  glDisableVertexAttribArray(index);
}

void VAO::attribPointer(unsigned int index, BufferObject& buffer, int size,
                        unsigned int type, unsigned char normalized, int stride,
                        void const* offset  // offset of the first component
) {
  buffer.bind();
  glVertexAttribPointer(index, size, type, normalized, stride, offset);
  // buffer.unbind();
}
