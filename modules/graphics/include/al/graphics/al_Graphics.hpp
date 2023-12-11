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
#include "al/graphics/al_Light.hpp"
#include "al/graphics/al_OpenGL.hpp"
#include "al/graphics/al_RenderManager.hpp"

namespace al {

/**
@defgroup Graphics Graphics
*/

/**
@brief Interface for loading fonts and rendering text
@ingroup Graphics
*/
class Graphics : public RenderManager {
public:
  enum class ColoringMode : unsigned int {
    UNIFORM,
    MESH,
    TEXTURE,
    MATERIAL,
    CUSTOM
  };

  virtual ~Graphics() {}

  /// buffer=[GL_NONE, GL_FRONT_LEFT, GL_FRONT_RIGHT, GL_BACK_LEFT,
  ///         GL_BACK_RIGHT, GL_FRONT, GL_BACK, GL_LEFT, GL_RIGHT,
  ///         GL_FRONT_AND_BACK]
  inline void bufferToDraw(unsigned int buffer) { gl::bufferToDraw(buffer); }

  /// Turn blending on/off
  inline void blending(bool doBlend) { gl::blending(doBlend); }
  /// Set blend mode
  /// src,dst=[GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR,
  ///          GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA,
  ///          GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
  ///          GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR,
  ///          GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA]
  /// eq=[GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT,
  ///     GL_MIN, GL_MAX]
  inline void blendMode(unsigned int src, unsigned int dst, unsigned int eq) {
    gl::blendMode(src, dst, eq);
  }
  /// Set blend mode to additive (symmetric additive lighten)
  inline void blendAdd() { gl::blendMode(GL_SRC_ALPHA, GL_ONE, GL_FUNC_ADD); }
  /// Set blend mode to subtractive (symmetric additive darken)
  inline void blendSub() {
    gl::blendMode(GL_SRC_ALPHA, GL_ONE, GL_FUNC_REVERSE_SUBTRACT);
  }
  /// Set blend mode to screen (symmetric multiplicative lighten)
  inline void blendScreen() {
    gl::blendMode(GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_FUNC_ADD);
  }
  /// Set blend mode to multiplicative (symmetric multiplicative darken)
  inline void blendMult() { gl::blendMode(GL_DST_COLOR, GL_ZERO, GL_FUNC_ADD); }
  /// Set blend mode to transparent (asymmetric)
  inline void blendTrans() {
    gl::blendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD);
  }

  /// Turn depth testing on/off
  inline void depthTesting(bool testDepth) { gl::depthTesting(testDepth); }
  /// Turn the depth mask on/off
  inline void depthMask(bool maskDepth) { gl::depthMask(maskDepth); }

  /// Turn scissor testing on/off
  inline void scissorTest(bool testScissor) { gl::scissorTest(testScissor); }
  inline void scissorArea(int left, int bottom, int width, int height) {
    gl::scissorArea(left, bottom, width, height);
  }

  /// Turn face culling on/off
  inline void culling(bool doCulling) { gl::culling(doCulling); }
  /// face=[GL_FRONT, GL_BACK, GL_FRONT_AND_BACK], initial: GL_BACK
  inline void cullFace(unsigned int face) { gl::cullFace(face); }
  inline void cullFaceBack() { gl::cullFace(GL_BACK); }
  inline void cullFaceFront() { gl::cullFace(GL_FRONT); }
  inline void cullFaceBoth() { gl::cullFace(GL_FRONT_AND_BACK); }

  /// Set polygon drawing mode
  /// mode=[GL_POINT, GL_LINE, GL_FILL]
  /// GL_FRONT and GL_BACK is deprecated in 3.2 core profile
  inline void polygonMode(unsigned int mode) { gl::polygonMode(mode); }
  /// Draw only points of vertices
  inline void polygonPoint() { gl::polygonMode(GL_POINT); }
  /// Draw only edges of polygons with lines
  inline void polygonLine() { gl::polygonMode(GL_LINE); }
  /// Draw filled polygons
  inline void polygonFill() { gl::polygonMode(GL_FILL); }

  /// Turn color mask RGBA components on/off
  inline void colorMask(bool r, bool g, bool b, bool a) {
    gl::colorMask(r, g, b, a);
  }
  /// Turn color mask on/off (all RGBA components)
  inline void colorMask(bool b) { gl::colorMask(b); }

  inline void pointSize(float size) { gl::pointSize(size); }
  inline void lineWidth(float size) { gl::lineWidth(size); }

  // clears the default color buffer(buffer 0) with the provided color
  inline void clearColor(float r, float g, float b, float a = 1.f) {
    gl::clearColor(r, g, b, a);
  }
  // clears color buffer using al::Color class
  inline void clearColor(Color const &c) { gl::clearColor(c.r, c.g, c.b, c.a); }

  // clears the depth buffer with the provided depth value
  inline void clearDepth(float d = 1.f) { gl::clearDepth(d); }

  // clears the specified color buffer with the provided color
  inline void clearBuffer(int buffer, float r, float g, float b,
                          float a = 1.f) {
    gl::clearBuffer(buffer, r, g, b, a);
  }

  // clears color & depth buffer with the provided color, and depth 1
  inline void clear(float r, float g, float b, float a = 1.f) {
    gl::clearColor(r, g, b, a);
    gl::clearDepth(1.f);
  }
  // clears color & depth buffer with grayscale values, and depth 1
  inline void clear(float grayscale = 0.f, float a = 1.f) {
    gl::clearColor(grayscale, grayscale, grayscale, a);
    gl::clearDepth(1.f);
  }
  // clears color & depth buffer using al::Color class, and depth 1
  inline void clear(Color const &c) { clear(c.r, c.g, c.b, c.a); }

  // extended, predefined render managing --------------------------------------
  void init();

  // set overall tint, regardless of rendering mode
  void tint(Color const &c);
  void tint(float r, float g, float b, float a = 1.f);
  void tint(float grayscale, float a = 1.f);

  // set to uniform color mode, using previously set uniform color
  void color();
  // set to uniform color mode, using provided color
  void color(float r, float g, float b, float a = 1.f);
  // set to uniform color mode, using provided color
  void color(Color const &c);
  // set to uniform color mode, using provided color
  void color(float grayscale, float a = 1.f);

  // set to mesh color mode, using mesh's color array
  void meshColor();

  // set to texture mode, using texture bound by user at location=0
  void texture();

  // set to material mode, using previously set material
  // if lighting is disabled, ColoringMode::COLOR will be used
  void material();

  // set to material mode, using provied material
  void material(Material const &m);

  // enable/disable lighting
  void lighting(bool b);

  // set number of lights used
  void numLight(int n);

  // turn on/off light at specified index.
  // if light is off it still gets calculated in the shader but zero is
  // multiplied as intensity. to prevent calculation, reorder lights and call
  // `numLight` function to change shader to one with less number of lights
  void enableLight(int idx);
  void disableLight(int idx);
  void toggleLight(int idx);

  // does not enable lighting, call lighting(true) to enable lighting
  void light(Light const &l, int idx = 0);

  void quad(Texture &tex, float x, float y, float w, float h,
            bool flip = false);
  void quadViewport(Texture &tex, float x = -1, float y = -1, float w = 2,
                    float h = 2);

  // use user made non-default shader. with this call user should set uniforms
  // manually (but stiil use allolib interface for mesh and model/view/proj
  // matrices)
  void shader(ShaderProgram &s);
  ShaderProgram &shader();
  ShaderProgram *shaderPtr();

  using RenderManager::camera; // makes camera(Viewpoint::SpecialType v)
                               // accessible
  void camera(Viewpoint const &v) override;

  void send_lighting_uniforms(ShaderProgram &s,
                              lighting_shader_uniforms const &u);
  void update() override;

  // to pass to the shader, combined with mLens.eyeSep(),
  // will set the uniform "eye_sep"
  static const float LEFT_EYE;
  static const float RIGHT_EYE;
  static const float MONO_EYE;

  void eye(float e);
  float eye() { return mEye; }

  Lens const &lens() const { return mLens; }
  Lens &lens();
  void lens(Lens const &l);

  void omni(bool b);
  bool omni();

private:
  bool initialized = false;

  Color mColor{1, 1, 1, 1};
  Color mTint{1, 1, 1, 1};

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

} // namespace al
#endif
