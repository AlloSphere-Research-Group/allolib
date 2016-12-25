// #include <stdlib.h>
#include "al/core/gl/al_Texture.hpp"
#include <iostream>

namespace al{

Texture::Texture() {
}

Texture::~Texture() {
  destroy();
}

void Texture::create2D(
  unsigned int _width, unsigned int _height,
  int _internal,
  unsigned int _format,
  unsigned int _type
) {
  mTarget = GL_TEXTURE_2D;
  mInternalFormat = _internal;
  mWidth = _width;
  mHeight = _height;
  mDepth = 1;
  mFormat = _format;
  mType = _type;

  create();
  bind();
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  glTexImage2D(
    target(),
    0, // level
    internalFormat(),
    width(), height(),
    0, // border
    format(), type(), NULL
  );
  unbind();
}

void Texture::onCreate() {
  glGenTextures(1, (GLuint *)&mID);
}

void Texture::onDestroy() {
  glDeleteTextures(1, (GLuint *)&mID);
}

void Texture::bind(int unit) {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(target(), id());
}

void Texture::unbind(int unit) {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(target(), 0);
}

Texture& Texture::filterMin(int v) {
  glTexParameteri(target(), GL_TEXTURE_MIN_FILTER, filterMin());
  return *this;
}

Texture& Texture::filterMag(int v) {
  glTexParameteri(target(), GL_TEXTURE_MAG_FILTER, filterMag());
  return *this;
}

Texture& Texture::wrapS(int v) {
  glTexParameteri(target(), GL_TEXTURE_WRAP_S, wrapS());
  return *this;
}

Texture& Texture::wrapT(int v) {
  glTexParameteri(target(), GL_TEXTURE_WRAP_T, wrapT());
  return *this;
}

Texture& Texture::wrapR(int v) {
  glTexParameteri(target(), GL_TEXTURE_WRAP_R, wrapR());
  return *this;
}

Texture& Texture::generateMipmap() {
  glGenerateMipmap(target());
  return *this;
}

void Texture::submit(const void * pixels) {
  if (!pixels) {
    return;
  }

  switch (target()) {
    case GL_TEXTURE_1D:
      glTexSubImage1D(
        target(), 0,
        0, width(),
        format(), type(), pixels
      );
      break;
    case GL_TEXTURE_2D:
      glTexSubImage2D(
        target(), 0,
        0, 0, width(), height(),
        format(), type(), pixels
      );
      break;
    case GL_TEXTURE_3D:
      glTexSubImage3D(
        target(), 0,
        0, 0, 0, width(), height(), depth(),
        format(), type(), pixels
      );
      break;
    default:
      AL_WARN("invalid texture target %d", target());
  }
}

} // al::
