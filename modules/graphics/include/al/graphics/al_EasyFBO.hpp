#ifndef __EASYFBO_HPP__
#define __EASYFBO_HPP__

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
  FBO helper, wrapper around fbo, rbo, texture, pose, projectionMatrix..

  File author(s):
  Tim Wood, 2015, fishuyo@gmail.com
  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/graphics/al_FBO.hpp"
#include "al/graphics/al_Texture.hpp"

namespace al {

struct EasyFBOSetting {
  // color attachment texture config
  int internal = GL_RGBA8;
  unsigned int format = GL_RGBA;
  unsigned int type = GL_UNSIGNED_BYTE;

  // format for depth attachment
  unsigned int depth_format = GL_DEPTH_COMPONENT24;

  // wrapping option for color texture
  int wrapS = GL_CLAMP_TO_EDGE;
  int wrapT = GL_CLAMP_TO_EDGE;
  int wrapR = GL_CLAMP_TO_EDGE;

  // filtering option for color texture
  int filterMin = GL_NEAREST;
  int filterMag = GL_NEAREST;

  bool mUseMipmap = false; // config mipmap levels for result texture?

  bool use_depth_texture = false; // if true, use texture with depth_format
                                  // for depth recording.
                                  // else use rbo for depth recording
};

/**
 * @brief Encapsulates FBO, depth buffer, and texture
 * @ingroup Graphics
 *
 * simple fbo + color attachment + depth attachment wrapper. Only supports one
 * color attaachment but user and attach more by getting fbo member object and
 * calling attach functions manually
 *
 */
class EasyFBO {
  int mWidth, mHeight;
  Texture mTex;
  Texture mDepthTex;
  RBO mRbo;
  FBO mFbo;

public:
  void init(int width, int height,
            EasyFBOSetting const &setting = EasyFBOSetting{});
  int width() { return mWidth; }
  int height() { return mHeight; }
  FBO &fbo() { return mFbo; }
  Texture &tex() { return mTex; }
  Texture &depthTex() { return mDepthTex; }
  RBO &rbo() { return mRbo; }
  void begin() { mFbo.bind(); }
  void end() { mFbo.unbind(); }
  void bind() { mFbo.bind(); }
};

} // namespace al

#endif
