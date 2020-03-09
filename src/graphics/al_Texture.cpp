#include "al/graphics/al_Texture.hpp"
#include "al/system/al_Printing.hpp"

// #include <stdlib.h>
#include <iostream>

namespace al {

Texture::Texture() {}

Texture::~Texture() { destroy(); }

void Texture::create1D(GLsizei width, GLint internal, GLenum format,
                       GLenum type) {
  mTarget = GL_TEXTURE_1D;
  mInternalFormat = internal;
  mWidth = width;
  mHeight = 1;
  mDepth = 1;
  mFormat = format;
  mType = type;

  // force sending params
  mFilterUpdated = true;
  mWrapUpdated = true;
  mUsingMipmapUpdated = true;

  glTexImage1D(mTarget, 0,                  // level
               mInternalFormat, mWidth, 0,  // border
               mFormat, mType, nullptr);

  update_filter();
  update_wrap();
  // no mipmap update since there's no data yet,
  // just flagging mUsingMipmapUpdated to be true
  unbind_temp();
}

void Texture::create2D(unsigned int width, unsigned int height, int internal,
                       unsigned int format, unsigned int type) {
  mTarget = GL_TEXTURE_2D;
  mInternalFormat = internal;
  mWidth = width;
  mHeight = height;
  mDepth = 1;
  mFormat = format;
  mType = type;

  // force sending params
  mFilterUpdated = true;
  mWrapUpdated = true;
  mUsingMipmapUpdated = true;

  // AL_GRAPHICS_ERROR("before creating 2D texture", id());
  create();
  bind_temp();
  glTexImage2D(mTarget,
               0,  // level
               mInternalFormat, mWidth, mHeight,
               0,  // border
               mFormat, mType, nullptr);
  // AL_GRAPHICS_ERROR("creating 2D texture", id());
  update_filter();
  update_wrap();
  // no mipmap update since there's no data yet,
  // just flagging mUsingMipmapUpdated to be true
  unbind_temp();
}

void Texture::create2DArray(unsigned int width, unsigned int height, unsigned int depth,  
                            int internal, unsigned int format, unsigned int type) {

  mTarget = GL_TEXTURE_2D_ARRAY,
  mInternalFormat = internal;
  mWidth = width;
  mHeight = height;
  mDepth = depth;
  mFormat = format;
  mType = type;

  // force sending params
  mFilterUpdated = true;
  mWrapUpdated = true;
  mUsingMipmapUpdated = true;

  create();
  bind_temp();
  glTexImage3D(mTarget,
             0,  // level
             mInternalFormat, mWidth, mHeight, mDepth,
             0,  // border
             mFormat, mType, nullptr);
  update_filter();
  update_wrap();
  unbind_temp();
}

void Texture::createCubemap(unsigned int size, int internal,
                            unsigned int format, unsigned int type) {
  mTarget = GL_TEXTURE_CUBE_MAP;
  mInternalFormat = internal;
  mWidth = size;
  mHeight = size;
  mDepth = 1;
  mFormat = format;
  mType = type;

  // force sending params
  mFilterUpdated = true;
  mWrapUpdated = true;
  mUsingMipmapUpdated = true;

  // AL_GRAPHICS_ERROR("before creating 2D texture", id());
  create();
  bind_temp();
  for (int i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,  //< target
                 0,                                   //< lod
                 mInternalFormat,                     //< internal format
                 mWidth, mWidth, 0,  //< equal throughout the faces
                 mFormat,            //< format of data
                 mType,              //< data type (e.g. GL_UNSIGNED_BYTE)
                 nullptr);           //< no actual data yet
  }
  update_filter();
  update_wrap();
  // no mipmap update since there's no data yet,
  // just flagging mUsingMipmapUpdated to be true
  unbind_temp();
}

void Texture::onCreate() { glGenTextures(1, (GLuint *)&mID); }

void Texture::onDestroy() { glDeleteTextures(1, (GLuint *)&mID); }

void Texture::bind(int binding_point) {
  // AL_GRAPHICS_ERROR("(before Texture::bind)", id());
  glActiveTexture(GL_TEXTURE0 + binding_point);
  glBindTexture(target(), id());
  // AL_GRAPHICS_ERROR("binding texture", id());

  // update params & mipmap if they were changed
  update_filter();
  update_wrap();
  update_mipmap();
}

void Texture::bind_temp() {
  glActiveTexture(GL_TEXTURE0 + AL_TEX_TEMP_BINDING_UNIT);
  glBindTexture(target(), id());
}

void Texture::unbind(int binding_point) { unbind(binding_point, target()); }

void Texture::unbind(int binding_point, unsigned int target) {
  glActiveTexture(GL_TEXTURE0 + binding_point);
  glBindTexture(target, 0);
}

void Texture::filterMin(int v) {
  switch (v) {
    case GL_NEAREST_MIPMAP_NEAREST:
    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
    case GL_LINEAR_MIPMAP_LINEAR:
    default:
      break;
  }
  update_param(v, mFilterMin, mFilterUpdated);
}

void Texture::filterMag(int v) {
  // no mipmap filtering for magnification,
  // so pick closest one if given
  switch (v) {
    case GL_NEAREST_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
      v = GL_NEAREST;
      break;
    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_LINEAR_MIPMAP_LINEAR:
      v = GL_LINEAR;
      break;
    default:
      break;
  }
  update_param(v, mFilterMag, mFilterUpdated);
}

void Texture::wrap(int S, int T, int R) {
  if (S != mWrapS || T != mWrapT || R != mWrapR) {
    mWrapS = S;
    mWrapT = T;
    mWrapR = R;
    mWrapUpdated = true;
  }
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
      // MAX LEVEL 1000 is default value of OpenGL
      glTexParameteri(target(), GL_TEXTURE_MAX_LEVEL, 1000);
      glGenerateMipmap(target());
    } else {
      // base level is 0 if untouched
      // glTexParameteri(target(), GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(target(), GL_TEXTURE_MAX_LEVEL, 0);
    }
    mUsingMipmapUpdated = false;
  }
}

void Texture::generateMipmap() {
  mUsingMipmapUpdated = true;
  mUseMipmap = true;
  // bind_temp();
  // glTexParameteri(target(), GL_TEXTURE_MAX_LEVEL, 1000);
  // glGenerateMipmap(target());
  // unbind_temp();
}

void Texture::disableMipmap() {
  if (mUseMipmap) {
    mUsingMipmapUpdated = true;
    mUseMipmap = false;
  }
  // bind_temp();
  // glTexParameteri(target(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(target(), GL_TEXTURE_MAX_LEVEL, 0);
  // unbind_temp();
}

// void Texture::update(bool force) {
//   bind_temp();
//   if (force) {
//     mFilterUpdated = true;
//     mWrapUpdated = true;
//     mUsingMipmapUpdated = true;
//   }
//   update_filter();
//   update_wrap();
//   update_mipmap();
//   unbind_temp();
// }

void Texture::submit(const void *pixels, unsigned int format,
                     unsigned int type) {
  if (!pixels) {
    return;
  }

  bind_temp();
  // AL_GRAPHICS_ERROR("before Texture::submit (glTexSubImage)", id());
  switch (target()) {
    case GL_TEXTURE_1D:
      glTexSubImage1D(target(), 0, 0, width(), format, type, pixels);
      break;
    case GL_TEXTURE_2D:
      glTexSubImage2D(target(), 0, 0, 0, width(), height(), format, type,
                      pixels);
      break;
    case GL_TEXTURE_2D_ARRAY:
    case GL_TEXTURE_3D:
      glTexSubImage3D(target(), 0, 0, 0, 0, width(), height(), depth(), format,
                      type, pixels);
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
  unbind_temp();
}

bool Texture::resize(unsigned int w, unsigned int h, int internalFormat,
                     unsigned int format, unsigned int type) {
  if (target() != GL_TEXTURE_2D) {
    AL_WARN("invalid geometry for texture 2D target");
    return false;
  }
  if (w == mWidth && h == mHeight) {
    // AL_WARN("same egometry");
    return false;
  }
  create2D(w, h, internalFormat, format, type);
  return true;
}

void Texture::copyFrameBuffer(int w, int h, int fbx, int fby, int texx,
                              int texy, int texz) {
  if (w < 0) {
    w += 1 + width();
  }
  if (h < 0) {
    h += 1 + height();
  }

  bind_temp();
  switch (target()) {
    case GL_TEXTURE_1D:
      glCopyTexSubImage1D(GL_TEXTURE_1D, 0, texx, fbx, fby, w);
      break;
    case GL_TEXTURE_2D:
      glCopyTexSubImage2D(GL_TEXTURE_2D, 0, texx, texy, fbx, fby, w, h);
      break;
    case GL_TEXTURE_3D:
      glCopyTexSubImage3D(GL_TEXTURE_3D, 0, texx, texy, texz, fbx, fby, w, h);
      break;
    default:;
  }
  unbind_temp();
}

int Texture::numComponents(Format v) {
  switch (v) {
    case RED:
      return 1;
    case RG:
      return 2;
    case RGB:
    case BGR:
      return 3;
    case RGBA:
    case BGRA:
      return 4;
    case DEPTH_COMPONENT:
      return 1;
    case DEPTH_STENCIL:
      return 2;
    default:
      return 0;
  };
}

}  // namespace al
