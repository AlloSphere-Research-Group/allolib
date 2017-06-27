#include "al/core/gl/al_Texture.hpp"
#include "al/core/system/al_Printing.hpp"

// #include <stdlib.h>
#include <iostream>
#include <unordered_map>

namespace al{

// keep track of textures bound 
std::unordered_map<int, Texture*>& boundTextures() {
  static std::unordered_map<int, Texture*> sBoundTextures;
  return sBoundTextures;
}

int get_empty_binding_point() {
  auto& t = boundTextures();
  for (int i = 0; i < AL_TEX_TEMP_BINDING_UNIT; i += 1) {
    auto search = t.find(i);
    if (search == t.end()) {
      // std::cout << "found empty binding point at " << i << std::endl;
      return i;
    }
  }
  // std::cout << "no more empty binding point" << std::endl;
  return -1;
}

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

  // bind above should have called makeActiveTexture
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
    bind(); 
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

    // bind above should have called makeActiveTexture
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

void Texture::makeActiveTexture() {
  glActiveTexture(GL_TEXTURE0 + mBindingPoint);
}

void Texture::bind() {
  int unit = get_empty_binding_point();
  bind(unit);
}

void Texture::bind(int unit) {
  // AL_GRAPHICS_ERROR("(before Texture::bind)", id());
  glActiveTexture(GL_TEXTURE0 + unit);
  auto& t = boundTextures();
  // unbind previously bound texture at this unit
  auto search = t.find(unit);
  if (search != t.end()) {
    (search->second)->mBindingPoint = -1;
    // std::cout << "unregistered texture at " << unit << std::endl;
  }
  glBindTexture(target(), id());
  t[unit] = this;
  mBindingPoint = unit;
  // std::cout << "bound texture at " << unit << std::endl;
  // AL_GRAPHICS_ERROR("binding texture", id());
}

void Texture::unbind() {
  makeActiveTexture();
  glBindTexture(target(), 0);
  mBindingPoint = -1;
}

int Texture::bindingPoint() {
  return mBindingPoint;
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
  makeActiveTexture();
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
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
      glGenerateMipmap(target());
    }
    else {
      // base level is 0 if untouched
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    }
    mUsingMipmapUpdated = false;
  }
}

void Texture::update(bool force) {
  if (force) {
    mFilterUpdated = true;
    mWrapUpdated = true;
    mUsingMipmapUpdated = true;
  }
  makeActiveTexture();
  update_filter();
  update_wrap();
  update_mipmap();
}

} // al::
