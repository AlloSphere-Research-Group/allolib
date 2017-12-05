#include "al/core/graphics/al_Graphics.hpp"

// #include <stdio.h>
// #include <iostream>

namespace al {

Color Graphics::mClearColor {0, 0, 0, 1};
float Graphics::mClearDepth = 1;
Color Graphics::mColor {1, 1, 1, 1};
Color Graphics::mTint {1, 1, 1, 1};

Graphics::ColoringMode Graphics::mColoringMode = ColoringMode::UNIFORM;
bool Graphics::mRenderModeChanged = true;
bool Graphics::mUniformChanged = true;
bool Graphics::mLightingEnabled = false;

ShaderProgram Graphics::mesh_shader;
ShaderProgram Graphics::color_shader;
ShaderProgram Graphics::tex_shader;

int Graphics::color_location = 0;
int Graphics::color_tint_location = 0;
int Graphics::mesh_tint_location = 0;
int Graphics::tex_tint_location = 0;

Material Graphics::mMaterial;
Light Graphics::mLights[al_max_num_lights()];
bool Graphics::mLightOn[al_max_num_lights()];
int Graphics::num_lights = 1;

ShaderProgram Graphics::lighting_color_shader[al_max_num_lights()];
ShaderProgram Graphics::lighting_mesh_shader[al_max_num_lights()];
ShaderProgram Graphics::lighting_tex_shader[al_max_num_lights()];
ShaderProgram Graphics::lighting_material_shader[al_max_num_lights()];

int Graphics::lighting_color_location[al_max_num_lights()];
int Graphics::lighting_color_tint_location[al_max_num_lights()];
int Graphics::lighting_mesh_tint_location[al_max_num_lights()];
int Graphics::lighting_tex_tint_location[al_max_num_lights()];
int Graphics::lighting_material_tint_location[al_max_num_lights()];

lighting_shader_uniforms Graphics::lighting_color_uniforms[al_max_num_lights()];
lighting_shader_uniforms Graphics::lighting_mesh_uniforms[al_max_num_lights()];
lighting_shader_uniforms Graphics::lighting_tex_uniforms[al_max_num_lights()];
lighting_shader_uniforms Graphics::lighting_material_uniforms[al_max_num_lights()];

void Graphics::blendMode(BlendFunc src, BlendFunc dst, BlendEq eq) {
  glBlendEquation(eq);
  glBlendFunc(src, dst);
}

void Graphics::capability(Capability cap, bool v) {
  v ? enable(cap) : disable(cap);
}

void Graphics::blending(bool b) { capability(BLEND, b); }
void Graphics::colorMask(bool r, bool g, bool b, bool a) {
  glColorMask(r ? GL_TRUE : GL_FALSE, g ? GL_TRUE : GL_FALSE,
              b ? GL_TRUE : GL_FALSE, a ? GL_TRUE : GL_FALSE);
}
void Graphics::colorMask(bool b) { colorMask(b, b, b, b); }
void Graphics::depthMask(bool b) { glDepthMask(b ? GL_TRUE : GL_FALSE); }
void Graphics::depthTesting(bool b) { capability(DEPTH_TEST, b); }
void Graphics::scissorTest(bool b) { capability(SCISSOR_TEST, b); }
void Graphics::cullFace(bool b) { capability(CULL_FACE, b); }
void Graphics::cullFace(bool b, Face face) {
  capability(CULL_FACE, b);
  glCullFace(face);
}

// void Graphics::lineWidth(float v) { glLineWidth(v); }
void Graphics::pointSize(float v) { glPointSize(v); }
void Graphics::polygonMode(PolygonMode m, Face f) { glPolygonMode(f, m); }

void Graphics::scissor(int left, int bottom, int width, int height) {
  glScissor(left, bottom, width, height);
}

void Graphics::setClearColor(float r, float g, float b, float a) {
  mClearColor.set(r, g, b, a);
}

void Graphics::setClearColor(Color const& c) { mClearColor = c; }

void Graphics::clearColorBuffer(int drawbuffer) {
  glClearBufferfv(GL_COLOR, drawbuffer, mClearColor.components);
}

void Graphics::clearColorBuffer(float r, float g, float b, float a, int drawbuffer) {
  setClearColor(r, g, b, a);
  clearColorBuffer(drawbuffer);
}

void Graphics::setClearDepth(float d) { mClearDepth = d; }

void Graphics::clearDepth() { glClearBufferfv(GL_DEPTH, 0, &mClearDepth); }

void Graphics::clearDepth(float d) {
  setClearDepth(d);
  clearDepth();
}

void Graphics::clearBuffer(int drawbuffer) {
  clearColorBuffer(drawbuffer);
  clearDepth();
}

void Graphics::clearBuffer(float r, float g, float b, float a, float d, int drawbuffer) {
  clearColorBuffer(r, g, b, a, drawbuffer);
  clearDepth(d);
}

void Graphics::init() {
  static bool initialized = false;
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
    compileMultiLightShader(lighting_color_shader[i], ShaderType::LIGHTING_COLOR, i + 1);
    compileMultiLightShader(lighting_mesh_shader[i], ShaderType::LIGHTING_MESH, i + 1);
    compileMultiLightShader(lighting_tex_shader[i], ShaderType::LIGHTING_TEXTURE, i + 1);
    compileMultiLightShader(lighting_material_shader[i], ShaderType::LIGHTING_MATERIAL, i + 1);

    lighting_color_location[i] = lighting_color_shader[i].getUniformLocation("col0");
    lighting_color_tint_location[i] = lighting_color_shader[i].getUniformLocation("tint");
    lighting_mesh_tint_location[i] = lighting_mesh_shader[i].getUniformLocation("tint");
    lighting_tex_tint_location[i] = lighting_tex_shader[i].getUniformLocation("tint");
    lighting_material_tint_location[i] = lighting_material_shader[i].getUniformLocation("tint");

    lighting_color_uniforms[i] = al_get_lighting_uniform_locations(lighting_color_shader[i]);
    lighting_mesh_uniforms[i] = al_get_lighting_uniform_locations(lighting_mesh_shader[i]);
    lighting_tex_uniforms[i] = al_get_lighting_uniform_locations(lighting_tex_shader[i]);
    lighting_material_uniforms[i] = al_get_lighting_uniform_locations(lighting_material_shader[i]);

    lighting_tex_shader[i].begin();
    lighting_tex_shader[i].uniform("tex0", 0);
    lighting_tex_shader[i].end();

    mLightOn[i] = true;
  }

  initialized = true;
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

void Graphics::enableLight(int idx) {
  mLightOn[idx] = true;
}
void Graphics::disableLight(int idx) {
  mLightOn[idx] = false;
}
void Graphics::toggleLight(int idx) {
  mLightOn[idx] = !mLightOn[idx];
}

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
  quad(tex, x, y, w, h);
  popCamera();
}

void Graphics::send_lighting_uniforms(ShaderProgram& s, lighting_shader_uniforms const& u) {
  s.uniform4v(u.global_ambient, Light::globalAmbient().components);
  s.uniformMatrix4(u.normal_matrix, (viewMatrix() * modelMatrix()).inversed().transpose().elems());
  for (int i = 0; i < u.num_lights; i += 1) {
      s.uniform4v(u.lights[i].ambient, mLights[i].ambient().components);
      s.uniform4v(u.lights[i].diffuse, mLights[i].diffuse().components);
      s.uniform4v(u.lights[i].specular, mLights[i].specular().components);
      s.uniform4v(u.lights[i].position, (viewMatrix() * Vec4f{mLights[i].pos()}).elems()); // could be optimized...
      s.uniform(u.lights[i].enabled, (mLightOn[i]? 1.0f : 0.0f)); // could be optimized...
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
        RenderManager::shader(mLightingEnabled ? lighting_color_shader[num_lights-1] : color_shader);
        break;
      case ColoringMode::MESH:
        RenderManager::shader(mLightingEnabled ? lighting_mesh_shader[num_lights-1] : mesh_shader);
        break;
      case ColoringMode::TEXTURE:
        RenderManager::shader(mLightingEnabled ? lighting_tex_shader[num_lights-1] : tex_shader);
        break;
      case ColoringMode::MATERIAL:
        RenderManager::shader(mLightingEnabled ? lighting_material_shader[num_lights-1] : color_shader);
        break;
      case ColoringMode::CUSTOM:
        // do nothing
        break;
    }
    mRenderModeChanged = false;
    mUniformChanged = true; // force uniform update since shader changed
  }

  if (mUniformChanged) {
    auto& s = RenderManager::shader();
    switch (mColoringMode) {
      case ColoringMode::UNIFORM:
        if (mLightingEnabled) {
          // send_uniforms(s, mLights[0]);
          send_lighting_uniforms(s, lighting_color_uniforms[num_lights-1]);
          s.uniform4v(lighting_color_location[num_lights-1], mColor.components);
          s.uniform4v(lighting_color_tint_location[num_lights-1], mTint.components);
        } else {
          s.uniform4v(color_location, mColor.components);
          s.uniform4v(color_tint_location, mTint.components);
        }
        break;
      case ColoringMode::MESH:
        if (mLightingEnabled) {
          // send_uniforms(s, mLights[0]);
          send_lighting_uniforms(s, lighting_mesh_uniforms[num_lights-1]);
          s.uniform4v(lighting_mesh_tint_location[num_lights-1], mTint.components);
        } else {
          s.uniform4v(mesh_tint_location, mTint.components);
        }
        break;
      case ColoringMode::TEXTURE:
        if (mLightingEnabled) {
          // send_uniforms(s, mLights[0]);
          send_lighting_uniforms(s, lighting_tex_uniforms[num_lights-1]);
          s.uniform4v(lighting_tex_tint_location[num_lights-1], mTint.components);
        } else {
          s.uniform4v(tex_tint_location, mTint.components);
        }
        break;
      case ColoringMode::MATERIAL:
        if (mLightingEnabled) {
          // send_uniforms(s, mMaterial);
          // send_uniforms(s, mLights[0]);
          send_lighting_uniforms(s, lighting_material_uniforms[num_lights-1]);
          s.uniform4v(lighting_material_tint_location[num_lights-1], mTint.components);
        } else {
          s.uniform4v(color_location, mColor.components);
          s.uniform4v(color_tint_location, mTint.components);
        }
        break;
      case ColoringMode::CUSTOM:
        // do nothing
       break;
    }
    mUniformChanged = false;
  }

  // also call base class's update
  RenderManager::update();
}

}  // namespace al
