#include "al/core/gl/al_Texture.hpp"
#include "al/core/system/al_Printing.hpp"

// #include <stdlib.h>
#include <iostream>

namespace al{

Texture::Texture() {
}

Texture::~Texture() {
  destroy();
}

void Texture::create2D(
  unsigned int width, unsigned int height,
  int internal,
  unsigned int format,
  unsigned int type)
{
  mTarget = GL_TEXTURE_2D;
  mInternalFormat = internal;
  mWidth = width;
  mHeight = height;
  mDepth = 1;
  mFormat = format;
  mType = type;

  // AL_GRAPHICS_ERROR("before creating 2D texture", id());
  create();
  bind();
  glTexImage2D(
    mTarget,
    0, // level
    mInternalFormat,
    mWidth, mHeight,
    0, // border
    mFormat, mType, NULL
  );
  // AL_GRAPHICS_ERROR("creating 2D texture", id());

  update(true); // true: force update
  unbind();
}

void Texture::onCreate() {
  glGenTextures(1, (GLuint *)&mID);
}

void Texture::onDestroy() {
  glDeleteTextures(1, (GLuint *)&mID);
}

void Texture::bind(int unit) {
  // AL_GRAPHICS_ERROR("(before Texture::bind)", id());
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(target(), id());
  // AL_GRAPHICS_ERROR("binding texture", id());
}

void Texture::unbind(int unit) {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(target(), 0);
}

void Texture::filterMin(int v) {
  switch(v) {
    case GL_NEAREST_MIPMAP_NEAREST:
    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
    case GL_LINEAR_MIPMAP_LINEAR:
      update_param(true, mUseMipmap, mUseMipmapUpdated);
      break;
    default:
      update_param(false, mUseMipmap, mUseMipmapUpdated);
  }
  update_param(v, mFilterMin, mParamsUpdated);
}

void Texture::filterMag(int v) {
  // no mipmap filtering for magnification,
  // so pick closest one if given
  switch(v) {
    case GL_NEAREST_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
      update_param(GL_NEAREST, mFilterMag, mParamsUpdated);
    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_LINEAR_MIPMAP_LINEAR:
      update_param(GL_LINEAR, mFilterMag, mParamsUpdated);
      break;
    default:
      update_param(v, mFilterMag, mParamsUpdated);
  }
}

void Texture::wrapS(int v) {
  update_param(v, mWrapS, mParamsUpdated);
}

void Texture::wrapT(int v) {
  update_param(v, mWrapT, mParamsUpdated);
}

void Texture::wrapR(int v) {
  update_param(v, mWrapR, mParamsUpdated);
}

void Texture::mipmap(bool b) {
  update_param(b, mUseMipmap, mUseMipmapUpdated);
}

void Texture::submit(const void * pixels) {
  if (!pixels) {
    return;
  }

  // AL_GRAPHICS_ERROR("before Texture::submit (glTexSubImage)", id());
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
  // AL_GRAPHICS_ERROR("Texture::submit (glTexSubImage)", id());

  // update mipmap with new data
  mUseMipmapUpdated = true;
}

void Texture::update(bool force) {
  if (mParamsUpdated || force) {
    glTexParameteri(target(), GL_TEXTURE_MAG_FILTER, filterMag());
    glTexParameteri(target(), GL_TEXTURE_MIN_FILTER, filterMin());
    glTexParameteri(target(), GL_TEXTURE_WRAP_S, wrapS());
    glTexParameteri(target(), GL_TEXTURE_WRAP_T, wrapT());
    glTexParameteri(target(), GL_TEXTURE_WRAP_R, wrapR());
    mParamsUpdated = false;
  }
  if (mUseMipmapUpdated || force) {
    if (mUseMipmap) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
      glGenerateMipmap(target());
    }
    else {
      // base level is 0 if untouched
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    }
    mUseMipmapUpdated = false;
  }
}

} // al::
