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

#include "al/graphics/al_GPUObject.hpp"
#include "al/graphics/al_OpenGL.hpp"
// #include "al/types/al_Color.hpp"

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

/*

usage:
Texture myTex;
myTex.filter(GL_LINEAR);
myTex.wrap(GL_CLAMP_TO_EDGE);
myTex.mipmap(false);
myTex.create2D(width, height, GL_RGB8, GL_RGBA, GL_FLOAT);

myTex.submit(pointer_to_data); // you can skip this if using the texture for
render target

to update  params:
myTex.filter(GL_LINEAR);
myTex.wrap(GL_CLAMP_TO_EDGE);
myTex.update(); // remember to call update, this is not needed when calling
create2D (internally called)

  frequently used internal format:
    GL_RGBA32F GL_RGBA8 GL_DEPTH_COMPONENT32F GL_DEPTH_COMPONENT16
*/

namespace al {

/// A simple wrapper around an OpenGL Texture
/// @ingroup Graphics
class Texture : public GPUObject {
 public:
  enum DataType : unsigned int /* GLenum */ {
    BYTE = GL_BYTE,             /**< */
    UBYTE = GL_UNSIGNED_BYTE,   /**< */
    SHORT = GL_SHORT,           /**< */
    USHORT = GL_UNSIGNED_SHORT, /**< */
    INT = GL_INT,               /**< */
    UINT = GL_UNSIGNED_INT,     /**< */
    FLOAT = GL_FLOAT,           /**< */
    DOUBLE = GL_DOUBLE          /**< */
  };

  enum Target : unsigned int /* GLenum */ {
    TEX_1D = GL_TEXTURE_1D,
    TEX_2D = GL_TEXTURE_2D,
    TEX_3D = GL_TEXTURE_3D,
    TEX_1D_ARRAY = GL_TEXTURE_1D_ARRAY,
    TEX_2D_ARRAY = GL_TEXTURE_2D_ARRAY,
    TEX_RECTANGLE = GL_TEXTURE_RECTANGLE,
    TEX_CUBE_MAP = GL_TEXTURE_CUBE_MAP,
    TEX_BUFFER = GL_TEXTURE_BUFFER,
    TEX_2D_MULTISAMPLE = GL_TEXTURE_2D_MULTISAMPLE,
    TEX_2D_MULTISAMPLE_ARRAY = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
    NO_TARGET = 0
  };

  enum Wrap : int /* GLint */ {
    CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
    CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
    MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
    REPEAT = GL_REPEAT
  };

  enum Filter : int /* GLint */ {
    NEAREST = GL_NEAREST,
    LINEAR = GL_LINEAR,
    // first term is within mipmap level, second term is between mipmap levels:
    NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
    LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
    NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
    LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR
  };

  enum Format : unsigned int /* GLenum */ {
    RED = GL_RED,
    RG = GL_RG,
    RGB = GL_RGB,
    BGR = GL_BGR,
    RGBA = GL_RGBA,
    BGRA = GL_BGRA,
    DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
    DEPTH_STENCIL = GL_DEPTH_STENCIL
  };

  enum Internal : int /* GLint */ {
    RGBA32F = GL_RGBA32F,
    RGBA16 = GL_RGBA16,
    RGBA16F = GL_RGBA16F,
    RGBA8 = GL_RGBA8,
    SRGB8_ALPHA8 = GL_SRGB8_ALPHA8,

    RG32F = GL_RG32F,
    RG16 = GL_RG16,
    RG16F = GL_RG16F,
    RG8 = GL_RG8,

    R32F = GL_R32F,
    R16F = GL_R16F,
    R8 = GL_R8,

    RGB32F = GL_RGB32F,
    RGB16F = GL_RGB16F,
    RGB16 = GL_RGB16,
    RGB8 = GL_RGB8,
    SRGB8 = GL_SRGB8,

    DEPTH_COMPONENT32F = GL_DEPTH_COMPONENT32F,
    DEPTH_COMPONENT24 = GL_DEPTH_COMPONENT24,
    DEPTH_COMPONENT16 = GL_DEPTH_COMPONENT16,
    DEPTH32F_STENCIL8 = GL_DEPTH32F_STENCIL8,
    DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8

    // there's more... but above are common ones
  };

  Texture();
  virtual ~Texture();

  void create1D(GLsizei width, GLint internal = GL_RGBA8,
                GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE);

  void create2D(unsigned int width, unsigned int height,
                int internal = GL_RGBA8, unsigned int format = GL_RGBA,
                unsigned int type = GL_UNSIGNED_BYTE  // or GL_FLOAT is used
  );

  void create2DArray(unsigned int width, unsigned int height, unsigned int depth,  
                int internal = GL_RGBA8, unsigned int format = GL_RGBA, 
                unsigned int type = GL_UNSIGNED_BYTE);
  // TODO
  // void create3D();

  void createCubemap(
      unsigned int size, int internal = GL_RGBA8, unsigned int format = GL_RGBA,
      unsigned int type = GL_UNSIGNED_BYTE  // or GL_FLOAT is used
  );

  /// Bind the texture (to a multitexture unit)
  /// also update params and mipmap if changed
  void bind(int binding_point = 0);
  /// use last binding point so it doesn't collide with user's binding
  /// also bind without updating params
  void bind_temp();

  /// Unbind the texture (from a multitexture unit)
  void unbind(int binding_point = 0);
  void unbind_temp() { unbind(AL_TEX_TEMP_BINDING_UNIT, target()); }
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

  /// Get number of components per pixel
  // unsigned numComponents() const { return Graphics::numComponents(format());
  // }

  /// Get total number of elements (components x width x height x depth)
  // unsigned numElems() const {
  //   return numPixels() * numComponents();
  // }

  /// Get total number of pixels
  // unsigned numPixels() const {
  //   return width() * (height()?height():1) * (depth()?depth():1);
  // }

  /// Resize 1D texture
  void resize(unsigned w) {}

  /// Resize 2D texture
  // void resize (unsigned w, unsigned h) { }
  bool resize(unsigned int w, unsigned int h, int internal, unsigned int format,
              unsigned int type);
  bool resize(unsigned int w, unsigned int h) {
    return resize(w, h, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
  }

  /// Resize 3D texture
  void resize(unsigned w, unsigned h, unsigned d) {}

  /// Set minification and magnification filter types all at once
  void filter(int v) {
    filterMin(v);
    filterMag(v);
  }

  /// Set minification filter type
  void filterMin(int v);

  /// Set magnification filter type
  void filterMag(int v);

  /// Set 3D wrapping modes
  void wrap(int S, int T, int R);

  /// Set 2D wrapping modes
  void wrap(int S, int T) { wrap(S, T, mWrapR); }

  /// Set wrapping mode for all dimensions
  void wrap(int v) { wrap(v, v, v); }

  void wrapS(int v) { wrap(v, mWrapT, mWrapR); }
  void wrapT(int v) { wrap(mWrapS, v, mWrapR); }
  void wrapR(int v) { wrap(mWrapS, mWrapT, v); }

  /// Set whether to generate mipmaps
  void mipmap(bool b) { mUseMipmap = b; }

  /// Copy client pixels to GPU texels
  /// NOTE: the graphics context (e.g. Window) must have been created
  /// If pixels is NULL, then the only effect is to resize the texture
  /// remotely.
  void submit(const void *pixels, unsigned int format, unsigned int type);
  void submit(const void *pixels) { submit(pixels, format(), type()); }

  // void submit(std::vector<Colori> const& pixels) {
  //   submit(pixels.data(), Texture::RGBA, Texture::UBYTE);
  // }
  // void submit(std::vector<Color> const& pixels) {
  //   submit(pixels.data(), Texture::RGBA, Texture::FLOAT);
  // }

  // update the changes in params or settings
  // void update(bool force=false);

  void generateMipmap();
  void disableMipmap();

  /// Copy pixels from current frame buffer to texture texels

  /// @param[in] w    width of region to copy; w<0 uses w + 1 + texture.width
  /// @param[in] h    height of region to copy; h<0 uses h + 1 + texture.height
  /// @param[in] fbx    pixel offset from left edge of frame buffer
  /// @param[in] fby    pixel offset from bottom edge of frame buffer
  /// @param[in] texx   texel offset in x direction
  /// @param[in] texy   texel offset in y direction (2D/3D only)
  /// @param[in] texz   texel offset in z direction (3D only)
  void copyFrameBuffer(int w = -1, int h = -1, int fbx = 0, int fby = 0,
                       int texx = 0, int texy = 0, int texz = 0);

  /// Returns number of components for given color type
  static int numComponents(Texture::Format v);

  /// Get number of components per pixel
  unsigned numComponents() const {
    return numComponents(Texture::Format(format()));
  }

 protected:
  void onCreate() override;
  void onDestroy() override;

  void update_filter();
  void update_wrap();
  void update_mipmap();

  // Pattern for setting a variable that when changed sets a notification flag
  // if v != var, update var and set flag to true
  template <class T>
  void update_param(const T &v, T &var, bool &flag) {
    if (v != var) {
      var = v;
      flag = true;
    }
  }

  unsigned int mTarget = GL_TEXTURE_2D;
  int mInternalFormat = GL_RGBA8;
  unsigned int mWidth = 0, mHeight = 0, mDepth = 0;
  unsigned int mFormat = GL_RGBA;
  unsigned int mType = GL_UNSIGNED_BYTE;

  int mWrapS = GL_CLAMP_TO_EDGE, mWrapT = GL_CLAMP_TO_EDGE,
      mWrapR = GL_CLAMP_TO_EDGE;
  int mFilterMin = GL_NEAREST, mFilterMag = GL_NEAREST;
  bool mUseMipmap = false;  // by default no mipmap

  bool mFilterUpdated = true;  // Flags change in texture params (wrap, filter)
  bool mWrapUpdated = true;    // Flags change in texture params (wrap, filter)
  bool mUsingMipmapUpdated = true;
};

}  // namespace al

#endif
