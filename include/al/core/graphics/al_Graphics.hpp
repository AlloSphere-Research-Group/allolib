#ifndef INCLUDE_AL_GRAPHICS_HPP
#define INCLUDE_AL_GRAPHICS_HPP

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
  A general high-level interface to graphics rendering

  File author(s):
  Wesley Smith, 2010, wesley.hoke@gmail.com
  Lance Putnam, 2010, putnam.lance@gmail.com
  Graham Wakefield, 2010, grrrwaaa@gmail.com
  Keehong Youn, 2017, younkeehong@gmail.com

*/

#include "al/core/graphics/al_DefaultShaders.hpp"
#include "al/core/graphics/al_OpenGL.hpp"
#include "al/core/graphics/al_RenderManager.hpp"

namespace al {

class Graphics : public RenderManager {
 public:
  enum BlendFunc : unsigned int {
    SRC_ALPHA = GL_SRC_ALPHA,                     /**< */
    ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA, /**< */
    SRC_COLOR = GL_SRC_COLOR,                     /**< */
    ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR, /**< */
    DST_ALPHA = GL_DST_ALPHA,                     /**< */
    ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA, /**< */
    DST_COLOR = GL_DST_COLOR,                     /**< */
    ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR, /**< */
    ZERO = GL_ZERO,                               /**< */
    ONE = GL_ONE,                                 /**< */
    SRC_ALPHA_SATURATE = GL_SRC_ALPHA_SATURATE    /**< */
  };

  enum BlendEq : unsigned int {
    FUNC_ADD = GL_FUNC_ADD,           /**< Source + destination */
    FUNC_SUBTRACT = GL_FUNC_SUBTRACT, /**< Source - destination */
    FUNC_REVERSE_SUBTRACT =
        GL_FUNC_REVERSE_SUBTRACT, /**< Destination - source */
    MIN = GL_MIN, /**< Minimum value of source and destination */
    MAX = GL_MAX  /**< Maximum value of source and destination */
  };

  enum Capability : unsigned int {
    BLEND = GL_BLEND, /**< Blend rather than replacing with new color */
    DEPTH_TEST = GL_DEPTH_TEST, /**< Test depth of incoming fragments */
    SCISSOR_TEST =
        GL_SCISSOR_TEST,     /**< Crop fragments according to scissor region */
    CULL_FACE = GL_CULL_FACE /**< Cull faces */
  };

  enum Face : unsigned int {
    FRONT = GL_FRONT,                  /**< Front face */
    BACK = GL_BACK,                    /**< Back face */
    FRONT_AND_BACK = GL_FRONT_AND_BACK /**< Front and back face */
  };

  enum PolygonMode : unsigned int {
    POINT = GL_POINT, /**< Render only points at each vertex */
    LINE = GL_LINE,   /**< Render only lines along vertex path */
    FILL = GL_FILL    /**< Render vertices normally according to primitive */
  };

  /// Enable a capability
  void enable(Capability v) { glEnable(v); }

  /// Disable a capability
  void disable(Capability v) { glDisable(v); }

  /// Set a capability
  void capability(Capability cap, bool value);

  /// Turn blending on/off
  void blending(bool b);

  /// Turn color mask RGBA components on/off
  void colorMask(bool r, bool g, bool b, bool a);

  /// Turn color mask on/off (all RGBA components)
  void colorMask(bool b);

  /// Turn the depth mask on/off
  void depthMask(bool b);

  /// Turn depth testing on/off
  void depthTesting(bool b);

  /// Turn scissor testing on/off
  void scissorTest(bool b);

  /// Turn face culling on/off
  void cullFace(bool b);

  /// Turn face culling on/off and set the culled face
  void cullFace(bool b, Face face);

  /// Set both line width and point diameter
  // void stroke(float v){ lineWidth(v); pointSize(v); }

  /// Set width, in pixels, of lines
  // void lineWidth(float v);

  /// Set diameter, in pixels, of points
  void pointSize(float v);

  /// Set polygon drawing mode
  void polygonMode(PolygonMode m, Face f = FRONT_AND_BACK);

  /// Draw only edges of polygons with lines
  void polygonLine(Face f = FRONT_AND_BACK) { polygonMode(LINE, f); }

  /// Draw filled polygons
  void polygonFill(Face f = FRONT_AND_BACK) { polygonMode(FILL, f); }

  /// Set blend mode
  void blendMode(BlendFunc src, BlendFunc dst, BlendEq eq = FUNC_ADD);

  /// Set blend mode to additive (symmetric additive lighten)
  void blendModeAdd() { blendMode(SRC_ALPHA, ONE, FUNC_ADD); }

  /// Set blend mode to subtractive (symmetric additive darken)
  void blendModeSub() { blendMode(SRC_ALPHA, ONE, FUNC_REVERSE_SUBTRACT); }

  /// Set blend mode to screen (symmetric multiplicative lighten)
  void blendModeScreen() { blendMode(ONE, ONE_MINUS_SRC_COLOR, FUNC_ADD); }

  /// Set blend mode to multiplicative (symmetric multiplicative darken)
  void blendModeMul() { blendMode(DST_COLOR, ZERO, FUNC_ADD); }

  /// Set blend mode to transparent (asymmetric)
  void blendModeTrans() { blendMode(SRC_ALPHA, ONE_MINUS_SRC_ALPHA, FUNC_ADD); }

  /// Turn blending states on (without setting mode)
  void blendOn() {
    depthMask(false);
    blending(true);
  }

  /// Set states for additive blending
  void blendAdd() {
    blendOn();
    blendModeAdd();
  }

  /// Set states for subtractive blending
  void blendSub() {
    blendOn();
    blendModeSub();
  }

  /// Set states for screen blending
  void blendScreen() {
    blendOn();
    blendModeScreen();
  }

  /// Set states for multiplicative blending
  void blendMul() {
    blendOn();
    blendModeMul();
  }

  /// Set states for transparent blending
  void blendTrans() {
    blendOn();
    blendModeTrans();
  }

  /// Turn blending states off (opaque rendering)
  void blendOff() {
    depthMask(true);
    blending(false);
  }

  void scissor(int left, int bottom, int width, int height);
  
  void setClearColor(float r, float g, float b, float a = 1);
  void clearColor(int drawbuffer = 0);
  void clearColor(float r, float g, float b, float a = 1, int drawbuffer = 0);
  void clearColor(Color const &c, int drawbuffer = 0);

  void setClearDepth(float d);
  void clearDepth();
  void clearDepth(float d);

  void clear(float r, float g, float b, float a = 1, float d = 1,
             int drawbuffer = 0);
  void clear(float grayscale, float d = 1) {
    clear(grayscale, grayscale, grayscale, 1, d);
  }
  void clear(Color const &c, float d = 1) { clear(c.r, c.g, c.b, c.a, d); }

  // extended render managing -----------------------------
  static void init();
  static void tint(float r, float g, float b, float a = 1.0f);
  static void tint(float grayscale, float a = 1.0f) {
    tint(grayscale, grayscale, grayscale, a);
  }
  static void color(float r, float g, float b, float a = 1.0f);
  static void color(float k, float a = 1.0f) { color(k, k, k, a); }
  static void color(Color const& c) { color(c.r, c.g, c.b, c.a); }
  static void bind(Texture& t);
  static void unbind();
  static void meshColor();

  void quad(Texture& tex, float x, float y, float w, float h);
  void quadViewport(Texture& tex, float x = -1, float y = -1, float w = 2,
                    float h = 2);

private:
  static bool initialized;
  static ShaderProgram mesh_shader;
  static ShaderProgram color_shader;
  static ShaderProgram tex_shader;
  static int color_location;
  static int color_tint_location;
  static int tex_tint_location;
  static int mesh_tint_location;
  static int tint_location;
  static Color tint_color;
  static Texture* texPtr;
  static Color mClearColor;
  static float mClearDepth;
};

}  // namespace al
#endif