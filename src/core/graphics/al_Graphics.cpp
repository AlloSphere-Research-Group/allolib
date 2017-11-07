#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/graphics/al_Shapes.hpp"

#include <stdio.h>
#include <iostream>

namespace al {

bool Graphics::initialized = false;
ShaderProgram Graphics::mesh_shader;
ShaderProgram Graphics::color_shader;
ShaderProgram Graphics::tex_shader;
int Graphics::color_location = 0;
int Graphics::color_tint_location = 0;
int Graphics::tex_tint_location = 0;
int Graphics::mesh_tint_location = 0;
int Graphics::tint_location = 0;
Color Graphics::tint_color{1.0f, 1.0f, 1.0f, 1.0f};
Texture* Graphics::texPtr = nullptr;
Color Graphics::mClearColor {0, 0, 0, 1};
float Graphics::mClearDepth = 1;

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

void Graphics::setClearColor(Color const& c) {
  mClearColor = c;
}

void Graphics::clearColor(int drawbuffer) {
  glClearBufferfv(GL_COLOR, drawbuffer, mClearColor.components);
}

void Graphics::clearColor(float r, float g, float b, float a,
                               int drawbuffer) {
  setClearColor(r, g, b, a);
  clearColor(drawbuffer);
}

void Graphics::clearColor(Color const& c, int drawbuffer) {
  setClearColor(c.r, c.g, c.b, c.a);
  clearColor(drawbuffer);
}

void Graphics::setClearDepth(float d) { mClearDepth = d; }

void Graphics::clearDepth() { glClearBufferfv(GL_DEPTH, 0, &mClearDepth); }

void Graphics::clearDepth(float d) {
  setClearDepth(d);
  clearDepth();
}

void Graphics::clear(float r, float g, float b, float a, float d, int drawbuffer) {
  clearColor(r, g, b, a, drawbuffer);
  clearDepth(d);
}


void Graphics::init() {
  if (initialized) return;
  compileDefaultShader(mesh_shader, ShaderType::MESH);
  compileDefaultShader(color_shader, ShaderType::COLOR);
  compileDefaultShader(tex_shader, ShaderType::TEXTURE);
  color_location = color_shader.getUniformLocation("col0");
  color_tint_location = color_shader.getUniformLocation("tint");
  tex_tint_location = tex_shader.getUniformLocation("tint");
  mesh_tint_location = mesh_shader.getUniformLocation("tint");
  tex_shader.begin();
  tex_shader.uniform("tex0", 0);
  tex_shader.uniform(tex_tint_location, 1, 1, 1, 1);
  tex_shader.end();
  color_shader.begin();
  color_shader.uniform(color_tint_location, 1, 1, 1, 1);
  color_shader.end();
  mesh_shader.begin();
  mesh_shader.uniform(mesh_tint_location, 1, 1, 1, 1);
  mesh_shader.end();
  shader(color_shader);
  tint_location = color_tint_location;
}

void Graphics::tint(float r, float g, float b, float a) {
  tint_color.set(r, g, b, a);
  shader().uniform4v(tint_location, tint_color.components);
}

void Graphics::color(float r, float g, float b, float a) {
  if (shader().id() != color_shader.id()) {
    shader(color_shader);
    tint_location = color_tint_location;
  }
  shader().uniform(color_location, r, g, b, a);
  shader().uniform4v(color_tint_location, tint_color.components);
}

void Graphics::bind(Texture& t) {
  if (shader().id() != tex_shader.id()) {
    shader(tex_shader);
    tint_location = tex_tint_location;
  }
  shader().uniform4v(tex_tint_location, tint_color.components);
  t.bind(0);
  texPtr = &t;
}

void Graphics::unbind() {
  texPtr->unbind(0);
  texPtr = nullptr;
}

void Graphics::meshColor() {
  if (shader().id() != mesh_shader.id()) {
    shader(mesh_shader);
    tint_location = mesh_tint_location;
  }
  shader().uniform4v(mesh_tint_location, tint_color.components);
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

  bind(tex);
  draw(m);
  unbind();
}

void Graphics::quadViewport(Texture& tex, float x, float y, float w, float h) {
  pushCamera();
  camera(Viewpoint::IDENTITY);
  quad(tex, x, y, w, h);
  popCamera();
}

}  // namespace al
