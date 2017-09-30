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
  bind_temp();
  glTexImage2D(
    mTarget,
    0, // level
    mInternalFormat,
    mWidth, mHeight,
    0, // border
    mFormat, mType, NULL
  );
  // AL_GRAPHICS_ERROR("creating 2D texture", id());
  mFilterUpdated = true;
  mWrapUpdated = true;
  mUsingMipmapUpdated = true;
  update_filter();
  update_wrap();
  update_mipmap();
}

void Texture::createCubemap(
    unsigned int size,
    int internal,
    unsigned int format,
    unsigned int type
) {
    mTarget = GL_TEXTURE_CUBE_MAP;
    mInternalFormat = internal;
    mWidth = size;
    mHeight = size;
    mDepth = 1;
    mFormat = format;
    mType = type;

    // AL_GRAPHICS_ERROR("before creating 2D texture", id());
    create();
    bind_temp();
    for (int i = 0; i < 6; i++) {
      glTexImage2D(
        GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, //< target
        0,                                //< lod
        mInternalFormat,                  //< internal format
        mWidth, mWidth, 0,                //< equal throughout the faces
        mFormat,                          //< format of data
        mType,                            //< data type (e.g. GL_UNSIGNED_BYTE)
        nullptr
      ); //< no actual data yet
    }
    mFilterUpdated = true;
    mWrapUpdated = true;
    mUsingMipmapUpdated = true;
    update_filter();
    update_wrap();
    update_mipmap();
}

void Texture::onCreate() {
  glGenTextures(1, (GLuint *)&mID);
}

void Texture::onDestroy() {
  glDeleteTextures(1, (GLuint *)&mID);
}

void Texture::bind(int binding_point) const {
  // AL_GRAPHICS_ERROR("(before Texture::bind)", id());
  glActiveTexture(GL_TEXTURE0 + binding_point);
  glBindTexture(target(), id());
  // AL_GRAPHICS_ERROR("binding texture", id());
}

void Texture::bind_temp() {
  bind(AL_TEX_TEMP_BINDING_UNIT);
}

void Texture::unbind(int binding_point) const {
  unbind(binding_point, target());
}

void Texture::unbind(int binding_point, unsigned int target) {
  glActiveTexture(GL_TEXTURE0 + binding_point);
  glBindTexture(target, 0);
}

void Texture::filterMin(int v) {
  switch(v) {
    case GL_NEAREST_MIPMAP_NEAREST:
    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
    case GL_LINEAR_MIPMAP_LINEAR:
      update_param(true, mUseMipmap, mUsingMipmapUpdated);
      break;
    default:
      update_param(false, mUseMipmap, mUsingMipmapUpdated);
  }
  update_param(v, mFilterMin, mFilterUpdated);
}

void Texture::filterMag(int v) {
  // no mipmap filtering for magnification,
  // so pick closest one if given
  switch(v) {
    case GL_NEAREST_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
      update_param(GL_NEAREST, mFilterMag, mFilterUpdated);
    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_LINEAR_MIPMAP_LINEAR:
      update_param(GL_LINEAR, mFilterMag, mFilterUpdated);
      break;
    default:
      update_param(v, mFilterMag, mFilterUpdated);
  }
}

void Texture::filter(int v) {
  switch(v) {
    case GL_NEAREST_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
      mFilterMag = GL_NEAREST;
      mUseMipmap = true;
      break;
    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_LINEAR_MIPMAP_LINEAR:
      mFilterMag = GL_LINEAR;
      mUseMipmap = true;
      break;
    default:
      mFilterMag = v;
      mUseMipmap = false;
  }
  mFilterMin = v;
  mFilterUpdated = true;
}

void Texture::wrapS(int v) {
  update_param(v, mWrapS, mWrapUpdated);
}

void Texture::wrapT(int v) {
  update_param(v, mWrapT, mWrapUpdated);
}

void Texture::wrapR(int v) {
  update_param(v, mWrapR, mWrapUpdated);
}

void Texture::wrap(int v) {
  mWrapS = v;
  mWrapT = v;
  mWrapR = v;
  mWrapUpdated = true;
}

void Texture::mipmap(bool b) {
  update_param(b, mUseMipmap, mUsingMipmapUpdated);
}

void Texture::submit(const void * pixels) {
  if (!pixels) {
    return;
  }
  bind_temp();
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
  mUsingMipmapUpdated = true;
  update_filter();
  update_wrap();
  update_mipmap();
}

void Texture::update_filter() {
  if (mFilterUpdated) {
    glTexParameteri(target(), GL_TEXTURE_MAG_FILTER, filterMag());
    glTexParameteri(target(), GL_TEXTURE_MIN_FILTER, filterMin());
    mFilterUpdated = false;
  }
}

void Texture::update_wrap() {
  if (mWrapUpdated) {
    glTexParameteri(target(), GL_TEXTURE_WRAP_S, wrapS());
    glTexParameteri(target(), GL_TEXTURE_WRAP_T, wrapT());
    glTexParameteri(target(), GL_TEXTURE_WRAP_R, wrapR());
    mWrapUpdated = false;
  }
}

void Texture::update_mipmap() {
  if (mUsingMipmapUpdated) {
    if (mUseMipmap) {
      // base level is 0 if untouched so no need to call
      // glTexParameteri(target(), GL_TEXTURE_BASE_LEVEL, 0);
      // 1000 is default value of OpenGL
      glTexParameteri(target(), GL_TEXTURE_MAX_LEVEL, 1000);
      glGenerateMipmap(target());
    }
    else {
      // base level is 0 if untouched
      // glTexParameteri(target(), GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(target(), GL_TEXTURE_MAX_LEVEL, 0);
    }
    mUsingMipmapUpdated = false;
  }
}

void Texture::update(bool force) {
  bind_temp();
  if (force) {
    mFilterUpdated = true;
    mWrapUpdated = true;
    mUsingMipmapUpdated = true;
  }
  update_filter();
  update_wrap();
  update_mipmap();
}

} // al::
