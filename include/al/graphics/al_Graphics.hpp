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

#include "al/graphics/al_DefaultShaders.hpp"
#include "al/graphics/al_OpenGL.hpp"
#include "al/graphics/al_RenderManager.hpp"
#include "al/graphics/al_Light.hpp"

namespace al {

class Graphics : public RenderManager {
public:

  enum [[deprecated("use GL_* enums with al::gl::* functions")]] BlendFunc : unsigned int {
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

  enum [[deprecated("use GL_* enums with al::gl::* functions")]] BlendEq : unsigned int {
    FUNC_ADD = GL_FUNC_ADD,           /**< Source + destination */
    FUNC_SUBTRACT = GL_FUNC_SUBTRACT, /**< Source - destination */
    FUNC_REVERSE_SUBTRACT =
        GL_FUNC_REVERSE_SUBTRACT, /**< Destination - source */
    MIN = GL_MIN, /**< Minimum value of source and destination */
    MAX = GL_MAX  /**< Maximum value of source and destination */
  };

  enum [[deprecated("use GL_* enums with al::gl::* functions")]] Capability : unsigned int {
    BLEND = GL_BLEND, /**< Blend rather than replacing with new color */
    DEPTH_TEST = GL_DEPTH_TEST, /**< Test depth of incoming fragments */
    SCISSOR_TEST =
        GL_SCISSOR_TEST,     /**< Crop fragments according to scissor region */
    CULL_FACE = GL_CULL_FACE /**< Cull faces */
  };

  enum [[deprecated("use GL_* enums with al::gl::* functions")]] Face : unsigned int {
    FRONT = GL_FRONT,                  /**< Front face */
    BACK = GL_BACK,                    /**< Back face */
    FRONT_AND_BACK = GL_FRONT_AND_BACK /**< Front and back face */
  };

  enum [[deprecated("use GL_* enums with al::gl::* functions")]] PolygonMode : unsigned int {
    POINT = GL_POINT, /**< Render only points at each vertex */
    LINE = GL_LINE,   /**< Render only lines along vertex path */
    FILL = GL_FILL    /**< Render vertices normally according to primitive */
  };

  enum class ColoringMode : unsigned int {
    UNIFORM,
    MESH,
    TEXTURE,
    MATERIAL,
    CUSTOM
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

  void scissor(int left, int bottom, int width, int height);
  
  void setClearColor(float r, float g, float b, float a = 1);
  void setClearColor(Color const& c);

  void clearColorBuffer(int drawbuffer);
  void clearColorBuffer(float r, float g, float b, float a, int drawbuffer);
  void clearColorBuffer(float k, float a, int drawbuffer) { clearColorBuffer(k, k, k, a, drawbuffer); }
  void clearColorBuffer(Color const &c, int drawbuffer) { clearColorBuffer(c.r, c.g, c.b, c.a, drawbuffer); }

  void clearColor() { clearColorBuffer(0); }
  void clearColor(float r, float g, float b, float a = 1) { clearColorBuffer(r, g, b, a, 0); }
  void clearColor(float k, float a = 1) { clearColorBuffer(k, k, k, a, 0); }
  void clearColor(Color const &c)  { clearColorBuffer(c.r, c.g, c.b, c.a, 0); }

  void setClearDepth(float d);
  void clearDepth();
  void clearDepth(float d);

  void clearBuffer(int drawbuffer);
  void clearBuffer(float r, float g, float b, float a, float d, int drawbuffer);
  void clearBuffer(float k, float a, float d, int drawbuffer) { clearBuffer(k, k, k, a, d, drawbuffer); }
  void clearBuffer(Color const &c, float d, int drawbuffer) { clearBuffer(c.r, c.g, c.b, c.a, d, drawbuffer); }

  void clear() { clearBuffer(0); }
  void clear(float r, float g, float b, float a = 1) { clearBuffer(r, g, b, a, 1, 0); }
  void clear(float k, float a = 1) { clearBuffer(k, k, k, a, 1, 0); }
  void clear(Color const &c) { clearBuffer(c.r, c.g, c.b, c.a, 1, 0); }

  // GL_NONE       /  GL_COLOR_ATTACHMENTn
  // GL_FRONT_LEFT /  GL_FRONT_RIGHT
  // GL_BACK_LEFT  /  GL_BACK_RIGHT
  void drawBuffer(unsigned int mode) {
    // glDrawBuffers(GLsizei n, GLenum const* bufs);
    glDrawBuffers(1, &mode);
  }

  // extended, predefined render managing --------------------------------------
  void init();

  // set overall tint, regardless of rendering mode
  void tint(Color const& c) { mTint = c; mUniformChanged = true; }
  void tint(float r, float g, float b, float a = 1.0f) {
    mTint.set(r, g, b, a);
    mUniformChanged = true;
  }
  void tint(float grayscale, float a = 1.0f) { tint(grayscale, grayscale, grayscale, a); }

  // set to uniform color mode, using previously set uniform color
  void color();
  // set to uniform color mode, using provided color
  void color(float r, float g, float b, float a = 1.0f);
  // set to uniform color mode, using provided color
  void color(Color const& c);
  // set to uniform color mode, using provided color
  void color(float k, float a = 1.0f) { color(k, k, k, a); }

  // set to mesh color mode, using mesh's color array
  void meshColor();

  // set to texture mode, using texture bound by user at location=0
  void texture();

  // set to material mode, using previously set material
  // if lighting is disabled, ColoringMode::COLOR will be used
  void material();

  // set to material mode, using provied material
  void material(Material const& m);

  // enable/disable lighting
  void lighting(bool b);

  // set number of lights used
  void numLight(int n);

  // turn on/off light at specified index.
  // if light is off it still gets calculated in the shader but zero is multiplied as intensity.
  // to prevent calculation, reorder lights and call `numLight` function
  // to change shader to one with less number of lights
  void enableLight(int idx);
  void disableLight(int idx);
  void toggleLight(int idx);

  // does not enable lighting, call lighting(true) to enable lighting
  void light(Light const& l, int idx=0);

  void quad(Texture& tex, float x, float y, float w, float h);
  void quadViewport(Texture& tex, float x = -1, float y = -1, float w = 2, float h = 2);

  // use user made non-default shader. with this call user should set uniforms manually
  // (but stiil use allolib interface for mesh and model/view/proj matrices)
  void shader(ShaderProgram& s) { mColoringMode = ColoringMode::CUSTOM; RenderManager::shader(s); }
  ShaderProgram& shader() { return RenderManager::shader(); }
  ShaderProgram* shaderPtr() { return RenderManager::shaderPtr(); }

  using RenderManager::camera; // makes camera(Viewpoint::SpecialType v) accessible
  void camera(Viewpoint const& v) override {
    mLens = v.lens();
    mUniformChanged = true;
    RenderManager::camera(v);
  }

  void send_lighting_uniforms(ShaderProgram& s, lighting_shader_uniforms const& u);
  void update() override;

  // to pass to the shader, combined with mLens.eyeSep(),
  // will set the uniform "eye_sep"
  static const float LEFT_EYE;
  static const float RIGHT_EYE;
  static const float MONO_EYE;
  void eye(float e) {
    mEye = e;
    mUniformChanged = true;
  }

  float eye() {
    return mEye;
  }

  Lens const& lens() const {
    return mLens;
  }

  Lens& lens() {
    mUniformChanged = true;
    return mLens;
  }

  void lens(Lens const& l) {
    mUniformChanged = true;
    mLens = l;
  }
  
  void omni(bool b) {
    is_omni = b;
    mRenderModeChanged = true;
  }
  
  bool omni() {
    return is_omni;
  }

  // deprecated
  [[deprecated]]
  void blendOn() { blending(true); }
  [[deprecated]]
  void blendAdd() {blendModeAdd(); }
  [[deprecated]]
  void blendSub() {blendModeSub(); }
  [[deprecated]]
  void blendScreen() {blendModeScreen(); }
  [[deprecated]]
  void blendMul() {blendModeMul(); }
  [[deprecated]]
  void blendTrans() {blendModeTrans(); }
  [[deprecated]]
  void blendOff() { blending(false); }

private:
  bool initialized = false;

  Color mClearColor {0, 0, 0, 1};
  float mClearDepth = 1;
  Color mColor {1, 1, 1, 1};
  Color mTint {1, 1, 1, 1};

  Graphics::ColoringMode mColoringMode = ColoringMode::UNIFORM;
  bool mRenderModeChanged = true;
  bool mUniformChanged = true;
  bool mLightingEnabled = false;

  ShaderProgram mesh_shader;
  ShaderProgram color_shader;
  ShaderProgram tex_shader;

  int color_location = 0;
  int color_tint_location = 0;
  int mesh_tint_location = 0;
  int tex_tint_location = 0;

  Material mMaterial;
  Light mLights[al_max_num_lights()];
  bool mLightOn[al_max_num_lights()];
  int num_lights = 1;

  ShaderProgram lighting_color_shader[al_max_num_lights()];
  ShaderProgram lighting_mesh_shader[al_max_num_lights()];
  ShaderProgram lighting_tex_shader[al_max_num_lights()];
  ShaderProgram lighting_material_shader[al_max_num_lights()];

  int lighting_color_location[al_max_num_lights()];
  int lighting_color_tint_location[al_max_num_lights()];
  int lighting_mesh_tint_location[al_max_num_lights()];
  int lighting_tex_tint_location[al_max_num_lights()];
  int lighting_material_tint_location[al_max_num_lights()];

  lighting_shader_uniforms lighting_color_uniforms[al_max_num_lights()];
  lighting_shader_uniforms lighting_mesh_uniforms[al_max_num_lights()];
  lighting_shader_uniforms lighting_tex_uniforms[al_max_num_lights()];
  lighting_shader_uniforms lighting_material_uniforms[al_max_num_lights()];

  bool is_omni = false;

  ShaderProgram omni_mesh_shader;
  ShaderProgram omni_color_shader;
  ShaderProgram omni_tex_shader;

  int omni_color_location = 0;
  int omni_color_tint_location = 0;
  int omni_mesh_tint_location = 0;
  int omni_tex_tint_location = 0;

  ShaderProgram omni_lighting_color_shader[al_max_num_lights()];
  ShaderProgram omni_lighting_mesh_shader[al_max_num_lights()];
  ShaderProgram omni_lighting_tex_shader[al_max_num_lights()];
  ShaderProgram omni_lighting_material_shader[al_max_num_lights()];

  int omni_lighting_color_location[al_max_num_lights()];
  int omni_lighting_color_tint_location[al_max_num_lights()];
  int omni_lighting_mesh_tint_location[al_max_num_lights()];
  int omni_lighting_tex_tint_location[al_max_num_lights()];
  int omni_lighting_material_tint_location[al_max_num_lights()];

  lighting_shader_uniforms omni_lighting_color_uniforms[al_max_num_lights()];
  lighting_shader_uniforms omni_lighting_mesh_uniforms[al_max_num_lights()];
  lighting_shader_uniforms omni_lighting_tex_uniforms[al_max_num_lights()];
  lighting_shader_uniforms omni_lighting_material_uniforms[al_max_num_lights()];

  Lens mLens;
  float mEye = 0.0f;
};

}  // namespace al
#endif
