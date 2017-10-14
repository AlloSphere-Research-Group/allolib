#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/graphics/al_Shapes.hpp"

#include <stdio.h>
#include <iostream>

namespace al {

Graphics::Graphics(Window* window): mWindowPtr(window) {}

void Graphics::blendMode(BlendFunc src, BlendFunc dst, BlendEq eq){
  glBlendEquation(eq);
  glBlendFunc(src, dst);
}

void Graphics::capability(Capability cap, bool v){
  v ? enable(cap) : disable(cap);
}

void Graphics::blending(bool b){ capability(BLEND, b); }
void Graphics::colorMask(bool r, bool g, bool b, bool a){
  glColorMask(
    r?GL_TRUE:GL_FALSE,
    g?GL_TRUE:GL_FALSE,
    b?GL_TRUE:GL_FALSE,
    a?GL_TRUE:GL_FALSE
  );
}
void Graphics::colorMask(bool b){ colorMask(b,b,b,b); }
void Graphics::depthMask(bool b){ glDepthMask(b?GL_TRUE:GL_FALSE); }
void Graphics::depthTesting(bool b){ capability(DEPTH_TEST, b); }
void Graphics::scissorTest(bool b){ capability(SCISSOR_TEST, b); }
void Graphics::cullFace(bool b){ capability(CULL_FACE, b); }
void Graphics::cullFace(bool b, Face face) {
  capability(CULL_FACE, b);
  glCullFace(face);
}

Matrix4f Graphics::modelMatrix() {
    return mModelStack.get();
}

Matrix4f Graphics::viewMatrix() {
  return mViewMat;
}

Matrix4f Graphics::projMatrix() {
  return mProjMat;
}

void Graphics::loadIdentity() { mModelStack.setIdentity(); }

void Graphics::pushMatrix(){
    mModelStack.push();
    mMatChanged = true;
}
void Graphics::popMatrix(){
    mModelStack.pop();
    mMatChanged = true;
}
void Graphics::translate(float x, float y, float z){
    mModelStack.mult(Matrix4f::translation(x, y, z));
    mMatChanged = true;
}
void Graphics::rotate(float angle, float x, float y, float z){
    mModelStack.mult(Matrix4f::rotate(angle, x, y, z));
    mMatChanged = true;
}
void Graphics::rotate(const Quatf& q) {
  Matrix4f m;
  q.toMatrix(m.elems());
  mModelStack.mult(m);
  mMatChanged = true;
}
void Graphics::scale(float s){
  scale(s, s, s);
}
void Graphics::scale(float x, float y, float z){
  mModelStack.mult(Matrix4f::scaling(x, y, z));
  mMatChanged = true;
}
// void Graphics::lineWidth(float v) { glLineWidth(v); }
void Graphics::pointSize(float v) { glPointSize(v); }
void Graphics::polygonMode(PolygonMode m, Face f){ glPolygonMode(f,m); }

void Graphics::setClearColor(float r, float g, float b, float a) {
    mClearColor.set(r, g, b, a);
}

void Graphics::clearColor(int drawbuffer) {
    glClearBufferfv(GL_COLOR, drawbuffer, mClearColor.components);
}

void Graphics::clearColor(float r, float g, float b, float a, int drawbuffer) {
    setClearColor(r, g, b, a);
    clearColor(drawbuffer);
}

void Graphics::clearColor(Color const& c, int drawbuffer) {
    setClearColor(c.r, c.g, c.b, c.a);
    clearColor(drawbuffer);
}

void Graphics::setClearDepth(float d) {
    mClearDepth = d;
}

void Graphics::clearDepth() {
    glClearBufferfv(GL_DEPTH, 0, &mClearDepth);
}

void Graphics::clearDepth(float d) {
    setClearDepth(d);
    clearDepth();
}

void Graphics::clear(float r, float g, float b, float a, float d, int drawbuffer) {
    clearColor(r, g, b, a, drawbuffer);
    clearDepth(d);
}

void Graphics::viewport(int left, int bottom, int width, int height) {
  glViewport(left, bottom, width, height);
}

void Graphics::scissor(int left, int bottom, int width, int height) {
  glScissor(left, bottom, width, height);
}

// void Graphics::scissor(const Viewport& v) {
//   glScissor(v.l, v.b, v.w, v.h);
// }

void Graphics::framebuffer(unsigned int id) {
  FBO::bind(id);
  mFBOID = id;
}

void Graphics::shader(ShaderProgram& s) {
  mShaderPtr = &s;
  mShaderPtr->use();
  mShaderChanged = true;
}

ShaderProgram& Graphics::shader() {
  return *mShaderPtr;
}

void Graphics::camera(Viewpoint const& v) {
  mViewMat = v.viewMatrix();
  mProjMat = v.projMatrix();
  auto const& vp = v.viewport();
  viewport(vp.l, vp.b, vp.w, vp.h, (mFBOID == 0)? mWindowPtr->highres_factor() : 1);
  mMatChanged = true;
}

void Graphics::camera(Viewpoint const& v, int w, int h) {
  camera(v, 0, 0, w, h);
}

void Graphics::camera(Viewpoint const& v, int x, int y, int w, int h) {
  mViewMat = v.viewMatrix();
  mProjMat = v.projMatrix(x, y, w, h);
  viewport(x, y, w, h, (mFBOID == 0)? mWindowPtr->highres_factor() : 1);
  mMatChanged = true;
}

void Graphics::camera(Viewpoint::SpecialType v) {
  camera(v, 0, 0, mWindowPtr->width(), mWindowPtr->height());
}

void Graphics::camera(Viewpoint::SpecialType v, int w, int h) {
  camera(v, 0, 0, w, h);
}

void Graphics::camera(Viewpoint::SpecialType v, int x, int y, int w, int h) {
  switch (v) {

  case Viewpoint::IDENTITY: {
    mViewMat = Matrix4f::identity();
    mProjMat = Matrix4f::identity();
  }
  break;

  case Viewpoint::ORTHO_FOR_2D: {
    // 1. place eye so that bottom left is (0, 0), top right is (width, height)
    // 2. set lens to be ortho, with given width and height
    float half_w = (w - x) * 0.5f;
    float half_h = (h - y) * 0.5f;

    // 2D things will be drawn at z = 0 >> z = 1, near: 0.5, far: 1.5
    mViewMat = Matrix4f::lookAt(
      Vec3f(half_w, half_h, 1), // eye
      Vec3f(half_w, half_h, 0), // at
      Vec3f(0, 1, 0) // up
    );
    mProjMat = Matrix4f::ortho(
      -half_w, half_w, // left, right
      -half_h, half_h, // bottom, top
      0.5f, 1.5f // near, far
    );
  }
  break;

  case Viewpoint::UNIT_ORTHO:
    float spanx = float(w) / h;
    float spany = 1;
    if (spanx < 1) {
      spanx = 1;
      spany = float(h) / w;
    }
    mViewMat = Matrix4f::identity();
    mProjMat = Matrix4f::ortho(
      -spanx, spanx, // left, right
      -spany, spany, // bottom, top
      -0.5f, 0.5f // near, far
    );
  break;
  }

  // viewport is in framebuffer unit
  viewport(x, y, w, h, (mFBOID == 0)? mWindowPtr->highres_factor() : 1);
  mMatChanged = true;
}

void Graphics::update() {
    if (mShaderChanged || mMatChanged) {
        shader().uniform("MV", viewMatrix() * modelMatrix());
        shader().uniform("P", projMatrix());
    }

    mShaderChanged = false;
    mMatChanged = false;
}

void Graphics::draw(VAOMesh& mesh) {
  update();
  mesh.draw();
}

void Graphics::draw(EasyVAO& vao) {
  update();
  vao.draw();
}

void Graphics::draw(Mesh& mesh) {
  // uses internal vao object.
  mInternalVAO.update(mesh);
  update();
  mInternalVAO.draw();
}

void Graphics::draw(Mesh&& mesh) {
  // uses internal vao object.
  mInternalVAO.update(mesh);
  update();
  mInternalVAO.draw();
}

} // al::
