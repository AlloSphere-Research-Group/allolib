#ifndef INCLUDE_AL_GRAPHICS_FBO_HPP
#define INCLUDE_AL_GRAPHICS_FBO_HPP

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
  Render and frame buffer object abstractions

  File author(s):
  Lance Putnam, 2012, putnam.lance@gmail.com
  Keehong Youn, 2017, younkeehong@gmail.com

*/

#include "al/graphics/al_GPUObject.hpp"
#include "al/graphics/al_OpenGL.hpp"
#include "al/graphics/al_Texture.hpp"

namespace al {

///
/// @ingroup allocore

/* supported format for renderbuffer
(not every format can be renderbuffer format)
GL_RGBA8, GL_RG8, GL_RED8
GL_RGBA16, GL_RG16, GL_RED16

GL_RGBA16F, GL_RG16F, GL_RED16F
GL_RGBA32F, GL_RG32F, GL_RED32F

GL_RGBA8I, GL_RG8I, GL_RED8I
GL_RGBA16I, GL_RG16I, GL_RED16I
GL_RGBA32I, GL_RG32I, GL_RED32I

GL_RGBA8UI, GL_RG8UI, GL_RED8UI
GL_RGBA16UI, GL_RG16UI, GL_RED16UI
GL_RGBA32UI, GL_RG32UI, GL_RED32UI

GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32F
GL_DEPTH24_STENCIL8, GL_DEPTH32F_STENCIL8
*/

/**
 * @brief Render buffer object class
 * @ingroup Graphics
 *
 * Render buffer objects are used for offscreen rendering. They are a single
 * image of a renderable internal format, such as color, depth, or stencil.
 */
class RBO : public GPUObject {
public:
  /// @param[in] format  internal format of buffer
  RBO(unsigned int format = GL_DEPTH_COMPONENT16);

  ~RBO() { destroy(); }

  /// Get internal pixel format
  unsigned int format() const;

  /// Set internal pixel format
  RBO &format(unsigned int v);

  /// Bind object
  void bind();

  /// Unbind object
  void unbind();

  /// Set dimensions, in pixels

  /// @param[in] width  width, in pixels
  /// @param[in] height  height, in pixels
  /// \returns whether the resize was successful
  bool resize(unsigned width, unsigned height);

  void create(unsigned int width, unsigned int height,
              unsigned int format = GL_DEPTH_COMPONENT16) {
    mFormat = format;
    resize(width, height);
  }

  /// Get maximum buffer size
  static unsigned maxSize();

  static void bind(unsigned id);
  static bool resize(unsigned int format, unsigned width, unsigned height);

protected:
  unsigned int mFormat;

  virtual void onCreate();
  virtual void onDestroy();
};

///

///
///
///
/**
 * @brief Frame buffer object class
 * @ingroup Graphics
 *
 * A frame buffer object is an application-created frame buffer that is
 * non-displayable. A single FBO can have multiple rendering destinations
 * (attachments) for color, depth, and stencil information. If the attachment
 * is a texture, then the FBO does "render-to-texture". If the attachment is
 * a render buffer object, then the FBO does "offscreen rendering".
 * A single FBO can have multiple color attachments, but only one depth and
 * one stencil attachment. Switching the attachments (attach/detach calls) is
 * much faster than using multiple FBOs.
 * All attachments must have the same dimensions and all color attachments
 * must have the same pixel format. These are standard requirements of an FBO,
 * not an implementation-imposed limitation.
 *
 */
class FBO : public GPUObject {
public:
  /// Attachment type
  enum Attachment {
    COLOR_ATTACHMENT0 = GL_COLOR_ATTACHMENT0,
    COLOR_ATTACHMENT1 = GL_COLOR_ATTACHMENT1,
    COLOR_ATTACHMENT2 = GL_COLOR_ATTACHMENT2,
    COLOR_ATTACHMENT3 = GL_COLOR_ATTACHMENT3,
    DEPTH_ATTACHMENT = GL_DEPTH_ATTACHMENT,
    STENCIL_ATTACHMENT = GL_STENCIL_ATTACHMENT,
    DEPTH_STENCIL_ATTACHMENT = GL_DEPTH_STENCIL_ATTACHMENT
  };

  ~FBO() { destroy(); }

  /// Attach RBO at specified attachment point
  FBO &attachRBO(const RBO &rbo, unsigned int attachment = GL_DEPTH_ATTACHMENT);

  /// Detach RBO at specified attachment point
  FBO &detachRBO(unsigned int attachment);

  /// Attach a texture

  /// @param[in] texID  texture ID
  /// @param[in] attach  Attachment type
  /// @param[in] level  mipmap level of texture
  FBO &attachTexture2D(Texture const &tex,
                       unsigned int attach = GL_COLOR_ATTACHMENT0,
                       int level = 0);

  /// Detach texture at a specified attachment point and mipmap level
  FBO &detachTexture2D(unsigned int attachment, int level = 0);

  FBO &attachCubemapFace(Texture const &tex, unsigned int target_face,
                         unsigned int attachment = GL_COLOR_ATTACHMENT0,
                         int level = 0);

  FBO &detachCubemapFace(unsigned int target_face, unsigned int attachment,
                         int level = 0);

  /// Bind object (start rendering to attached objects)
  void bind();

  /// Unbind object
  void unbind();

  void begin() { bind(); }
  void end() { unbind(); }

  /// Get status of frame buffer object
  GLenum status();
  const char *statusString();
  const char *statusString(GLenum stat);

  static void bind(unsigned fboID);
  static void renderBuffer(unsigned rboID, unsigned int attachment);
  static void texture2D(unsigned texID,
                        unsigned int attachment = GL_COLOR_ATTACHMENT0,
                        int level = 0);
  static void textureCubemapFace(unsigned int texID, unsigned int target_face,
                                 unsigned int attachment = GL_COLOR_ATTACHMENT0,
                                 int level = 0);

protected:
  virtual void onCreate();
  virtual void onDestroy();

public:
  static unsigned int const DEFAULT{0};
};

} // namespace al
#endif
