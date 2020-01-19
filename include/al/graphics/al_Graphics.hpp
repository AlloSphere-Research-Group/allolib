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

  // clears color buffer using al::Color class
  void clearColor(Color const& c) { gl::clearColor(c.r, c.g, c.b, c.a); }

  // clears color & depth buffer
  void clear(float r = 0, float g = 0, float b = 0, float a = 1) {
    gl::clearColor(r, g, b, a);
    gl::clearDepth(1);
  }

  // clears color & depth buffer using al::Color class
  void clear(Color const& c) { clear(c.r, c.g, c.b, c.a); }

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
  void tint(Color const& c) {
    mTint = c;
    mUniformChanged = true;
  }
  void tint(float r, float g, float b, float a = 1.0f) {
    mTint.set(r, g, b, a);
    mUniformChanged = true;
  }
  void tint(float grayscale, float a = 1.0f) {
    tint(grayscale, grayscale, grayscale, a);
  }

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
  // if light is off it still gets calculated in the shader but zero is
  // multiplied as intensity. to prevent calculation, reorder lights and call
  // `numLight` function to change shader to one with less number of lights
  void enableLight(int idx);
  void disableLight(int idx);
  void toggleLight(int idx);

  // does not enable lighting, call lighting(true) to enable lighting
  void light(Light const& l, int idx = 0);

  void quad(Texture& tex, float x, float y, float w, float h);
  void quadViewport(Texture& tex, float x = -1, float y = -1, float w = 2,
                    float h = 2);

  // use user made non-default shader. with this call user should set uniforms
  // manually (but stiil use allolib interface for mesh and model/view/proj
  // matrices)
  void shader(ShaderProgram& s) {
    mColoringMode = ColoringMode::CUSTOM;
    RenderManager::shader(s);
  }
  ShaderProgram& shader() { return RenderManager::shader(); }
  ShaderProgram* shaderPtr() { return RenderManager::shaderPtr(); }

  using RenderManager::camera;  // makes camera(Viewpoint::SpecialType v)
                                // accessible
  void camera(Viewpoint const& v) override {
    mLens = v.lens();
    mUniformChanged = true;
    RenderManager::camera(v);
  }

  void send_lighting_uniforms(ShaderProgram& s,
                              lighting_shader_uniforms const& u);
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

  float eye() { return mEye; }

  Lens const& lens() const { return mLens; }

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

  bool omni() { return is_omni; }

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

}  // namespace al
#endif
