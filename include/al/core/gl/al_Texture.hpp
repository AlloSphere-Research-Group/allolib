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
  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/gl/al_GPUObject.hpp"
#include "al/core/gl/al_GLEW.hpp"

/*
https://www.khronos.org/opengl/wiki/Shader#Resource_limitations
GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
The total number of texture units that can be used from all active programs.
This is the limit on glActiveTexture(GL_TEXTURE0 + i) and glBindSampler.
In GL 3.3, this was 48; in 4.3, it is 96.
*/
#define AL_TEX_MAX_BINDING_UNIT 48
// for temporary internal binding such as creating a texture
#define AL_TEX_TEMP_BINDING_UNIT 47
// for drawing quad slabs with texture in al::Graphics
#define AL_TEX_QUAD_DRAW_BINDING_UNIT 46

/*

usage:
Texture myTex;
myTex.filter(GL_LINEAR);
myTex.wrap(GL_CLAMP_TO_EDGE);
myTex.mipmap(false);
myTex.create2D(width, height, GL_RGB8, GL_RGBA, GL_FLOAT);

myTex.submit(pointer_to_data); // you can skip this if using the texture for render target

to update  params:
myTex.filter(GL_LINEAR);
myTex.wrap(GL_CLAMP_TO_EDGE);
myTex.update(); // remember to call update, this is not needed when calling create2D (internally called)

*/

/*
  
  frequently used internal format
  
  GL_RGBA32F
  GL_RGBA8

  GL_DEPTH_COMPONENT32F
  GL_DEPTH_COMPONENT16

*/

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

  int mWrapS = GL_CLAMP_TO_EDGE, mWrapT = GL_CLAMP_TO_EDGE, mWrapR = GL_CLAMP_TO_EDGE;
  int mFilterMin = GL_NEAREST, mFilterMag = GL_NEAREST;
  bool mUseMipmap = false; // by default no mipmap

  bool mFilterUpdated = true; // Flags change in texture params (wrap, filter)
  bool mWrapUpdated = true; // Flags change in texture params (wrap, filter)
  bool mUsingMipmapUpdated = true;;

public:
  Texture();
  virtual ~Texture();

  // TODO
  // void create1D();

  void create2D(
    unsigned int width, unsigned int height,
    int internal = GL_RGBA8,
    unsigned int format = GL_RGBA,
    unsigned int type = GL_UNSIGNED_BYTE // or GL_FLOAT is used
  );

  // TODO
  // void create3D();

  void createCubemap(
    unsigned int size,
    int internal = GL_RGBA8,
    unsigned int format = GL_RGBA,
    unsigned int type = GL_UNSIGNED_BYTE // or GL_FLOAT is used
  );


  /// Bind the texture (to a multitexture unit)
  void bind(int binding_point = 0);
  /// use lasts binding point for temporary binding
  /// so doesn't (most of the time) collide with user's binding
  void bind_temp();

  /// Unbind the texture (from a multitexture unit)
  void unbind(int binding_point = 0);
  static void unbind(int binding_point, unsigned int target);

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

  /// Set minification and magnification filter types all at once
  void filter(int v);

  /// Set minification filter type
  void filterMin(int v);

  /// Set magnification filter type
  void filterMag(int v);

  /// Set wrapping mode for all dimensions
  void wrap(int v);

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
  void submit(const void * pixels);

  // update the changes in params or settings
  void update(bool force=false);
  // should call makeActiveTexture before calling these
  void update_filter();
  void update_wrap();
  void update_mipmap();

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
