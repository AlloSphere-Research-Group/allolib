#ifndef INCLUDE_AL_GRAPHICS_TEXTURE_HPP
#define INCLUDE_AL_GRAPHICS_TEXTURE_HPP

/*  Allocore --
  Multimedia / virtual environment application class library

  Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
  Copyright (C) 2012. The Regents of the University of California.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    Neither the name of the University of California nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.


  File description:
  Helper object for Graphics Textures

  File author(s):
  Lance Putnam, 2015, putnam.lance@gmail.com
  Graham Wakefield, 2010, grrrwaaa@gmail.com
  Wesley Smith, 2010, wesley.hoke@gmail.com
*/

#include "al/core/gl/al_GPUObject.hpp"
#include "al/core/gl/al_Graphics.hpp"

namespace al {

/// A simple wrapper around an OpenGL Texture
/// @ingroup allocore
class Texture : public GPUObject {
protected:
  unsigned int mTarget;
  int mInternalFormat;
  unsigned int mWidth, mHeight, mDepth;
  unsigned int mFormat;
  unsigned int mType;

  int mWrapS, mWrapT, mWrapR;
  int mFilterMin, mFilterMag;
  bool mUseMipmap;

  bool mParamsUpdated; // Flags change in texture params (wrap, filter)
  bool mUseMipmapUpdated;

public:
  Texture();
  virtual ~Texture();

  // TODO
  // void create1D();

  void create2D(
    unsigned int _width, unsigned int _height,
    int internal = GL_RGBA8,
    unsigned int format = GL_RGBA,
    unsigned int type = GL_UNSIGNED_BYTE
  );

  // TODO
  // void create3D();

  /// Bind the texture (to a multitexture unit)
  void bind(int unit = 0);

  /// Unbind the texture (from a multitexture unit)
  void unbind(int unit = 0);

  /// Get target type (e.g., TEXTURE_2D)
  unsigned int target() const { return mTarget; }

  /// Get internal format
  int internalFormat() const { return mInternalFormat; }

  /// Get pixel (color) format
  unsigned int format() const { return mFormat; }

  /// Get pixel component data type
  unsigned int type() const { return mType; }

  /// Get width, in pixels
  unsigned int width() const { return mWidth; }

  /// Get height, in pixels
  unsigned int height() const { return mHeight; }

  /// Get depth, in pixels
  unsigned int depth() const { return mDepth; }

  /// Get minification filter type
  int filterMin() const { return mFilterMin; }

  /// Get magnification filter type
  int filterMag() const { return mFilterMag; }

  /// Get S wrapping type
  int wrapS() const { return mWrapS; }

  /// Get T wrapping type
  int wrapT() const { return mWrapT; }
  
  /// Get R wrapping type
  int wrapR() const { return mWrapR; }

  bool mipmap() const { return mUseMipmap; }

  /// Set minification and magnification filter types
  void filter(int v){ filterMin(v); filterMag(v); }

  /// Set minification filter type
  void filterMin(int v);

  /// Set magnification filter type
  void filterMag(int v);

  /// Set wrapping mode for all dimensions
  void wrap(int v){ return wrap(v,v,v); }

  void wrapS(int v);
  void wrapT(int v);
  void wrapR(int v);

  /// Set 2D wrapping modes
  void wrap(int S, int T){ wrapS(S); wrapT(T); }

  /// Set 3D wrapping modes
  void wrap(int S, int T, int R) {
    wrapS(S); wrapT(T); wrapR(R);
  };

  /// Set whether to generate mipmaps
  void mipmap(bool b);

  /// Copy client pixels to GPU texels

  /// NOTE: the graphics context (e.g. Window) must have been created
  /// If pixels is NULL, then the only effect is to resize the texture
  /// remotely.
  /// give unit if wanted a specific binding point to be used
  /// -1 for unit if don't want to bind internally (manual binding from outside)
  void submit(const void * pixels);

  // update the changes in params or settings
  void update(bool force=false);

protected:
  virtual void onCreate() override;
  virtual void onDestroy() override;

  // Pattern for setting a variable that when changed sets a notification flag
  // if v != var, update var and set flag to true
  template<class T>
  void update_param(const T& v, T& var, bool& flag){
    if(v!=var){ var=v; flag=true; }
  }
};

} // al::

#endif
