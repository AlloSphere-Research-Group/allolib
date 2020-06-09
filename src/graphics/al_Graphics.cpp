#include "al/graphics/al_Graphics.hpp"

// #include <stdio.h>
// #include <iostream>

namespace al {

const float Graphics::LEFT_EYE = -1.0f;
const float Graphics::RIGHT_EYE = 1.0f;
const float Graphics::MONO_EYE = 0.0f;

void Graphics::init() {
  if (initialized) return;

  compileDefaultShader(color_shader, ShaderType::COLOR);
  compileDefaultShader(mesh_shader, ShaderType::MESH);
  compileDefaultShader(tex_shader, ShaderType::TEXTURE);

  color_location = color_shader.getUniformLocation("col0");
  color_tint_location = color_shader.getUniformLocation("tint");
  tex_tint_location = tex_shader.getUniformLocation("tint");
  mesh_tint_location = mesh_shader.getUniformLocation("tint");

  tex_shader.begin();
  tex_shader.uniform("tex0", 0);
  tex_shader.end();

  for (int i = 0; i < al_max_num_lights(); i += 1) {
    compileMultiLightShader(lighting_color_shader[i],
                            ShaderType::LIGHTING_COLOR, i + 1);
    compileMultiLightShader(lighting_mesh_shader[i], ShaderType::LIGHTING_MESH,
                            i + 1);
    compileMultiLightShader(lighting_tex_shader[i],
                            ShaderType::LIGHTING_TEXTURE, i + 1);
    compileMultiLightShader(lighting_material_shader[i],
                            ShaderType::LIGHTING_MATERIAL, i + 1);

    lighting_color_location[i] =
        lighting_color_shader[i].getUniformLocation("col0");
    lighting_color_tint_location[i] =
        lighting_color_shader[i].getUniformLocation("tint");
    lighting_mesh_tint_location[i] =
        lighting_mesh_shader[i].getUniformLocation("tint");
    lighting_tex_tint_location[i] =
        lighting_tex_shader[i].getUniformLocation("tint");
    lighting_material_tint_location[i] =
        lighting_material_shader[i].getUniformLocation("tint");

    lighting_color_uniforms[i] =
        al_get_lighting_uniform_locations(lighting_color_shader[i]);
    lighting_mesh_uniforms[i] =
        al_get_lighting_uniform_locations(lighting_mesh_shader[i]);
    lighting_tex_uniforms[i] =
        al_get_lighting_uniform_locations(lighting_tex_shader[i]);
    lighting_material_uniforms[i] =
        al_get_lighting_uniform_locations(lighting_material_shader[i]);

    lighting_tex_shader[i].begin();
    lighting_tex_shader[i].uniform("tex0", 0);
    lighting_tex_shader[i].end();
  }

  compileDefaultShader(omni_color_shader, ShaderType::COLOR, true);
  compileDefaultShader(omni_mesh_shader, ShaderType::MESH, true);
  compileDefaultShader(omni_tex_shader, ShaderType::TEXTURE, true);

  omni_color_location = omni_color_shader.getUniformLocation("col0");
  omni_color_tint_location = omni_color_shader.getUniformLocation("tint");
  omni_tex_tint_location = omni_tex_shader.getUniformLocation("tint");
  omni_mesh_tint_location = omni_mesh_shader.getUniformLocation("tint");

  omni_tex_shader.begin();
  omni_tex_shader.uniform("tex0", 0);
  omni_tex_shader.end();

  for (int i = 0; i < al_max_num_lights(); i += 1) {
    compileMultiLightShader(omni_lighting_color_shader[i],
                            ShaderType::LIGHTING_COLOR, i + 1, true);
    compileMultiLightShader(omni_lighting_mesh_shader[i],
                            ShaderType::LIGHTING_MESH, i + 1, true);
    compileMultiLightShader(omni_lighting_tex_shader[i],
                            ShaderType::LIGHTING_TEXTURE, i + 1, true);
    compileMultiLightShader(omni_lighting_material_shader[i],
                            ShaderType::LIGHTING_MATERIAL, i + 1, true);

    omni_lighting_color_location[i] =
        omni_lighting_color_shader[i].getUniformLocation("col0");
    omni_lighting_color_tint_location[i] =
        omni_lighting_color_shader[i].getUniformLocation("tint");
    omni_lighting_mesh_tint_location[i] =
        omni_lighting_mesh_shader[i].getUniformLocation("tint");
    omni_lighting_tex_tint_location[i] =
        omni_lighting_tex_shader[i].getUniformLocation("tint");
    omni_lighting_material_tint_location[i] =
        omni_lighting_material_shader[i].getUniformLocation("tint");

    omni_lighting_color_uniforms[i] =
        al_get_lighting_uniform_locations(lighting_color_shader[i]);
    omni_lighting_mesh_uniforms[i] =
        al_get_lighting_uniform_locations(lighting_mesh_shader[i]);
    omni_lighting_tex_uniforms[i] =
        al_get_lighting_uniform_locations(lighting_tex_shader[i]);
    omni_lighting_material_uniforms[i] =
        al_get_lighting_uniform_locations(lighting_material_shader[i]);

    omni_lighting_tex_shader[i].begin();
    omni_lighting_tex_shader[i].uniform("tex0", 0);
    omni_lighting_tex_shader[i].end();
  }

  for (int i = 0; i < al_max_num_lights(); i += 1) {
    mLightOn[i] = true;
  }

  initialized = true;
}

void Graphics::tint(const Color& c) {
  mTint = c;
  mUniformChanged = true;
}

void Graphics::tint(float r, float g, float b, float a) {
  mTint.set(r, g, b, a);
  mUniformChanged = true;
}

void Graphics::tint(float grayscale, float a) {
  tint(grayscale, grayscale, grayscale, a);
}

void Graphics::color() {
  if (mColoringMode != ColoringMode::UNIFORM) {
    mColoringMode = ColoringMode::UNIFORM;
    mRenderModeChanged = true;
  }
}

void Graphics::color(float r, float g, float b, float a) {
  mColor.set(r, g, b, a);
  mUniformChanged = true;
  color();
}

void Graphics::color(Color const& c) {
  mColor = c;
  mUniformChanged = true;
  color();
}

void Graphics::color(float grayscale, float a) {
  color(grayscale, grayscale, grayscale, a);
}

void Graphics::meshColor() {
  if (mColoringMode != ColoringMode::MESH) {
    mColoringMode = ColoringMode::MESH;
    mRenderModeChanged = true;
  }
}

void Graphics::texture() {
  if (mColoringMode != ColoringMode::TEXTURE) {
    mColoringMode = ColoringMode::TEXTURE;
    mRenderModeChanged = true;
  }
}

void Graphics::material() {
  if (mColoringMode != ColoringMode::MATERIAL) {
    mColoringMode = ColoringMode::MATERIAL;
    mRenderModeChanged = true;
  }
}
// set to material mode, using provied material
void Graphics::material(Material const& m) {
  mMaterial = m;
  mUniformChanged = true;
  material();
}

// enable/disable lighting
void Graphics::lighting(bool b) {
  if (mLightingEnabled != b) {
    mLightingEnabled = b;
    mRenderModeChanged = true;
  }
}

void Graphics::numLight(int n) {
  // if lighting on, should update change in light number
  // else it will get updated later when lighting gets enabled
  if (mLightingEnabled) mRenderModeChanged = true;
  num_lights = n;
}

// does not enable light, call lighting(true) to enable lighting
void Graphics::light(Light const& l, int idx) {
  mLights[idx] = l;
  // if lighting on, should update change in light info
  // else it will get updated later when lighting gets enabled
  if (mLightingEnabled) mUniformChanged = true;
  // change shader only if current number of light is less than given index
  if (num_lights <= idx) numLight(idx + 1);
}

void Graphics::enableLight(int idx) { mLightOn[idx] = true; }
void Graphics::disableLight(int idx) { mLightOn[idx] = false; }
void Graphics::toggleLight(int idx) { mLightOn[idx] = !mLightOn[idx]; }

void Graphics::quad(Texture& tex, float x, float y, float w, float h) {
  static Mesh m = []() {
    Mesh m{Mesh::TRIANGLE_STRIP};
    m.vertex(0, 0, 0);
    m.vertex(0, 0, 0);
    m.vertex(0, 0, 0);
    m.vertex(0, 0, 0);
    m.texCoord(0, 0);
    m.texCoord(1, 0);
    m.texCoord(0, 1);
    m.texCoord(1, 1);
    return m;
  }();

  auto& verts = m.vertices();
  verts[0].set(x, y, 0);
  verts[1].set(x + w, y, 0);
  verts[2].set(x, y + h, 0);
  verts[3].set(x + w, y + h, 0);

  tex.bind(0);
  texture();
  draw(m);
  tex.unbind(0);
}

void Graphics::quadViewport(Texture& tex, float x, float y, float w, float h) {
  pushCamera();
  camera(Viewpoint::IDENTITY);
  bool prev_lighting = mLightingEnabled;
  lighting(false);
  quad(tex, x, y, w, h);
  lighting(prev_lighting);  // put back previous lighting mode
  popCamera();
}

void Graphics::shader(ShaderProgram& s) {
  mColoringMode = ColoringMode::CUSTOM;
  RenderManager::shader(s);
}

ShaderProgram& Graphics::shader() { return RenderManager::shader(); }

ShaderProgram* Graphics::shaderPtr() { return RenderManager::shaderPtr(); }

void Graphics::camera(const Viewpoint& v) {
  mLens = v.lens();
  mUniformChanged = true;
  RenderManager::camera(v);
}

void Graphics::send_lighting_uniforms(ShaderProgram& s,
                                      lighting_shader_uniforms const& u) {
  s.uniform4v(u.global_ambient, Light::globalAmbient().components);
  s.uniformMatrix4(
      u.normal_matrix,
      (viewMatrix() * modelMatrix()).inversed().transpose().elems());
  for (int i = 0; i < u.num_lights; i += 1) {
    s.uniform4v(u.lights[i].ambient, mLights[i].ambient().components);
    s.uniform4v(u.lights[i].diffuse, mLights[i].diffuse().components);
    s.uniform4v(u.lights[i].specular, mLights[i].specular().components);
    s.uniform4v(u.lights[i].position, (viewMatrix() * Vec4f{mLights[i].pos()})
                                          .elems());  // could be optimized...
    s.uniform(u.lights[i].enabled,
              (mLightOn[i] ? 1.0f : 0.0f));  // could be optimized...
    // s.uniform4v(u.lights[i].atten, mLights[i].attenuation());
  }

  if (u.has_material) {
    s.uniform4v(u.material.ambient, mMaterial.ambient().components);
    s.uniform4v(u.material.diffuse, mMaterial.diffuse().components);
    s.uniform4v(u.material.specular, mMaterial.specular().components);
    s.uniform(u.material.shininess, mMaterial.shininess());
    // s.uniform4v(u.material.emission, mMaterial.emission().components);
  }
}

void Graphics::update() {
  if (mRenderModeChanged) {
    switch (mColoringMode) {
      case ColoringMode::UNIFORM:
        if (!is_omni)
          RenderManager::shader(mLightingEnabled
                                    ? lighting_color_shader[num_lights - 1]
                                    : color_shader);
        else
          RenderManager::shader(mLightingEnabled
                                    ? omni_lighting_color_shader[num_lights - 1]
                                    : omni_color_shader);
        break;
      case ColoringMode::MESH:
        if (!is_omni)
          RenderManager::shader(mLightingEnabled
                                    ? lighting_mesh_shader[num_lights - 1]
                                    : mesh_shader);
        else
          RenderManager::shader(mLightingEnabled
                                    ? omni_lighting_mesh_shader[num_lights - 1]
                                    : omni_mesh_shader);
        break;
      case ColoringMode::TEXTURE:
        if (!is_omni)
          RenderManager::shader(mLightingEnabled
                                    ? lighting_tex_shader[num_lights - 1]
                                    : tex_shader);
        else
          RenderManager::shader(mLightingEnabled
                                    ? omni_lighting_tex_shader[num_lights - 1]
                                    : omni_tex_shader);
        break;
      case ColoringMode::MATERIAL:
        if (!is_omni)
          RenderManager::shader(mLightingEnabled
                                    ? lighting_material_shader[num_lights - 1]
                                    : color_shader);
        else
          RenderManager::shader(
              mLightingEnabled ? omni_lighting_material_shader[num_lights - 1]
                               : omni_color_shader);
        break;
      case ColoringMode::CUSTOM:
        // do nothing
        break;
    }
    mRenderModeChanged = false;
    mUniformChanged = true;  // force uniform update since shader changed
  }

  /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

      BIG TODO: All uniform handling below can be simplified with uniform buffer
     object (UBO) for now a lot of conditionals are used. Other possible
     solution would be using index (but less preferred than UBO) ex) shader[type
     + num_type * lighting + num_type * num_lighting * omni]
  */

  if (mUniformChanged) {
    auto& s = RenderManager::shader();
    switch (mColoringMode) {
      case ColoringMode::UNIFORM:
        if (mLightingEnabled) {
          if (!is_omni) {
            send_lighting_uniforms(s, lighting_color_uniforms[num_lights - 1]);
            s.uniform4v(lighting_color_location[num_lights - 1],
                        mColor.components);
            s.uniform4v(lighting_color_tint_location[num_lights - 1],
                        mTint.components);
          } else {
            send_lighting_uniforms(
                s, omni_lighting_color_uniforms[num_lights - 1]);
            s.uniform4v(omni_lighting_color_location[num_lights - 1],
                        mColor.components);
            s.uniform4v(omni_lighting_color_tint_location[num_lights - 1],
                        mTint.components);
          }
        } else {
          if (!is_omni) {
            s.uniform4v(color_location, mColor.components);
            s.uniform4v(color_tint_location, mTint.components);
          } else {
            s.uniform4v(omni_color_location, mColor.components);
            s.uniform4v(omni_color_tint_location, mTint.components);
          }
        }
        break;
      case ColoringMode::MESH:
        if (mLightingEnabled) {
          if (!is_omni) {
            send_lighting_uniforms(s, lighting_mesh_uniforms[num_lights - 1]);
            s.uniform4v(lighting_mesh_tint_location[num_lights - 1],
                        mTint.components);
          } else {
            send_lighting_uniforms(s,
                                   omni_lighting_mesh_uniforms[num_lights - 1]);
            s.uniform4v(omni_lighting_mesh_tint_location[num_lights - 1],
                        mTint.components);
          }
        } else {
          if (!is_omni) {
            s.uniform4v(mesh_tint_location, mTint.components);
          } else {
            s.uniform4v(omni_mesh_tint_location, mTint.components);
          }
        }
        break;
      case ColoringMode::TEXTURE:
        if (mLightingEnabled) {
          if (!is_omni) {
            send_lighting_uniforms(s, lighting_tex_uniforms[num_lights - 1]);
            s.uniform4v(lighting_tex_tint_location[num_lights - 1],
                        mTint.components);
          } else {
            send_lighting_uniforms(s,
                                   omni_lighting_tex_uniforms[num_lights - 1]);
            s.uniform4v(omni_lighting_tex_tint_location[num_lights - 1],
                        mTint.components);
          }
        } else {
          if (!is_omni)
            s.uniform4v(tex_tint_location, mTint.components);
          else
            s.uniform4v(omni_tex_tint_location, mTint.components);
        }
        break;
      case ColoringMode::MATERIAL:
        if (mLightingEnabled) {
          if (!is_omni) {
            send_lighting_uniforms(s,
                                   lighting_material_uniforms[num_lights - 1]);
            s.uniform4v(lighting_material_tint_location[num_lights - 1],
                        mTint.components);
          } else {
            send_lighting_uniforms(
                s, omni_lighting_material_uniforms[num_lights - 1]);
            s.uniform4v(omni_lighting_material_tint_location[num_lights - 1],
                        mTint.components);
          }
        } else {
          if (!is_omni) {
            s.uniform4v(color_location, mColor.components);
            s.uniform4v(color_tint_location, mTint.components);
          } else {
            s.uniform4v(omni_color_location, mColor.components);
            s.uniform4v(omni_color_tint_location, mTint.components);
          }
        }
        break;
      case ColoringMode::CUSTOM:
        // do nothing
        break;
    }

    // for any default shaders, needs to be cleaned up with other uniforms
    // (using pre saved location, or possibly uniform buffer)
    if (mColoringMode != ColoringMode::CUSTOM) {
      s.uniform("eye_sep", mLens.eyeSep() * mEye / 2.0f);
      s.uniform("foc_len", mLens.focalLength());
    }
    mUniformChanged = false;
  }

  // also call base class's update
  RenderManager::update();
}

void Graphics::eye(float e) {
  mEye = e;
  mUniformChanged = true;
}

Lens& Graphics::lens() {
  mUniformChanged = true;
  return mLens;
}

void Graphics::lens(const Lens& l) {
  mUniformChanged = true;
  mLens = l;
}

void Graphics::omni(bool b) {
  is_omni = b;
  mRenderModeChanged = true;
}

bool Graphics::omni() { return is_omni; }

}  // namespace al
