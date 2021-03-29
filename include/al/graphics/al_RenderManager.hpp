#ifndef INCLUDE_AL_RENDER_MANAGER_HPP
#define INCLUDE_AL_RENDER_MANAGER_HPP

#include <unordered_map>

#include "al/graphics/al_EasyFBO.hpp"
#include "al/graphics/al_EasyVAO.hpp"
#include "al/graphics/al_FBO.hpp"
#include "al/graphics/al_Shader.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/graphics/al_Viewpoint.hpp"
#include "al/math/al_Matrix4.hpp"
#include "al/math/al_Quat.hpp"
#include "al/math/al_Vec.hpp"

namespace al {

/**
@brief MatrixStack class
@ingroup Graphics
*/
class MatrixStack {
public:
  MatrixStack();
  void push();
  void pop();
  void mult(Matrix4f const &m);
  void set(const Matrix4f &m);
  void setIdentity();
  void pop_all();
  Matrix4f get() const;

private:
  std::vector<Matrix4f> stack;
};

/**
@brief ViewportStack class
@ingroup Graphics
*/
class ViewportStack {
public:
  ViewportStack();
  void push();
  void pop();
  Viewport get() const;
  void set(const Viewport &m);
  void set(int left, int bottom, int width, int height);

private:
  std::vector<Viewport> stack;
};

class FBOStack {
public:
  FBOStack();
  void push();
  void pop();
  unsigned int get() const;
  void set(unsigned int id);

private:
  std::vector<unsigned int> stack;
};

/**
 * @brief RenderManager class
 * @ingroup Graphics
 *
 * Render manager handles basic rendering states
 * 1. model / view / projection matrix
 *  - model matrix can be directly set
 *  - or can be set with translate / rotate / scale functions
 *  - view and projection matrix can be directly set
 *  - or can be set using Viewpoint class and RenderManager::camera function
 *  - matrices can be push / pop'ed
 *  - pushing and popping camera will push/pop both iew/proj mat
 * 2. viewport and framebuffer
 *  - keeps track of current state
 *  - also provides push / pop functionality
 * 3. shaders
 *  - keeps track of current shader
 *  - get & cache location and set values of uniforms for model, view,
projection matrix
 * 4. drawing mesh
 *  - sending vertex position/color/normal/texcoord to bound shader
 *  - mesh can be regular cpu-side al::Mesh
 *  - or gpu-stored al::VAOMesh
 *
 * !. writing shader for al::RenderManager
 *  - modelview matrix:  uniform mat4 l_ModelViewMatrix;
 *  - projection matrix: uniform mat4 al_ProjectionMatrix;
 *  - vertex position    layout (location = 0) in vec3 position;
 *  - vertex color:      layout (location = 1) in vec4 color;
 *  - vertex texcoord:   layout (location = 2) in vec2 texcoord;
 *  - vertex normal:     layout (location = 3) in vec3 normal;
*/
class RenderManager {
public:
  /// Multiply current matrix
  void multModelMatrix(const Matrix4f &m) {
    mModelStack.mult(m);
    mMatChanged = true;
  }
  void multViewMatrix(const Matrix4f &m) {
    mViewStack.mult(m);
    mMatChanged = true;
  }
  void multProjMatrix(const Matrix4f &m) {
    mProjStack.mult(m);
    mMatChanged = true;
  }

  Matrix4f modelMatrix() const { return mModelStack.get(); }
  Matrix4f viewMatrix() const { return mViewStack.get(); }
  Matrix4f projMatrix() const { return mProjStack.get(); }

  void modelMatrix(const Matrix4f &m) {
    mModelStack.set(m);
    mMatChanged = true;
  }
  void viewMatrix(const Matrix4f &m) {
    mViewStack.set(m);
    mMatChanged = true;
  }
  void projMatrix(const Matrix4f &m) {
    mProjStack.set(m);
    mMatChanged = true;
  }

  void pushModelMatrix() { mModelStack.push(); }
  void pushViewMatrix() { mViewStack.push(); }
  void pushProjMatrix() { mProjStack.push(); }

  void pushModelMatrix(const Matrix4f &m) {
    mModelStack.push();
    modelMatrix(m);
  }
  void pushViewMatrix(const Matrix4f &m) {
    mViewStack.push();
    viewMatrix(m);
  }
  void pushProjMatrix(const Matrix4f &m) {
    mProjStack.push();
    projMatrix(m);
  }

  void popModelMatrix() {
    mModelStack.pop();
    mMatChanged = true;
  }
  void popViewMatrix() {
    mViewStack.pop();
    mMatChanged = true;
  }
  void popProjMatrix() {
    mProjStack.pop();
    mMatChanged = true;
  }

  void resetModelMatrixStack() {
    mModelStack.pop_all();
    mMatChanged = true;
  }
  void resetViewMatrixStack() {
    mViewStack.pop_all();
    mMatChanged = true;
  }
  void resetProjMatrixStack() {
    mProjStack.pop_all();
    mMatChanged = true;
  }
  void resetMatrixStack() {
    resetModelMatrixStack();
    resetViewMatrixStack();
    resetProjMatrixStack();
  }

  /// Push current matrix stack
  void pushMatrix() { pushModelMatrix(); }
  void pushMatrix(const Matrix4f &m) { pushModelMatrix(m); }

  /// Pop current matrix stack
  void popMatrix() { popModelMatrix(); }

  /// Set current matrix to identity
  void loadIdentity() { mModelStack.setIdentity(); }

  /// Translate current matrix
  void translate(float x, float y, float z = 0.);
  /// Translate current matrix
  template <class T> void translate(const Vec<3, T> &v) {
    translate(float(v[0]), float(v[1]), float(v[2]));
  }
  /// Translate current matrix
  template <class T> void translate(const Vec<2, T> &v) {
    translate(v[0], v[1]);
  }

  /// Rotate current matrix
  /// \param[in] angle  angle, in degrees
  /// \param[in] x    x component of rotation axis
  /// \param[in] y    y component of rotation axis
  /// \param[in] z    z component of rotation axis
  void rotate(float angle, float x = 0., float y = 0., float z = 1.);
  /// Rotate current matrix
  void rotate(const Quatf &q);
  /// Rotate current matrix
  void rotate(const Quatd &q);
  /// Rotate current matrix
  /// \param[in] angle  angle, in degrees
  /// \param[in] axis   rotation axis
  template <class T> void rotate(float angle, const Vec<3, T> &axis) {
    rotate(angle, axis[0], axis[1], axis[2]);
  }

  /// Scale current matrix along each dimension
  void scale(float x, float y, float z = 1.);
  /// Scale current matrix uniformly
  void scale(float s) { scale(s, s, s); }
  /// Scale current matrix along each dimension
  template <class T> void scale(const Vec<3, T> &v) { scale(v[0], v[1], v[2]); }
  /// Scale current matrix along each dimension
  template <class T> void scale(const Vec<2, T> &v) { scale(v[0], v[1]); }

  /// Set viewport
  void viewport(int left, int bottom, int width, int height);
  void viewport(const Viewport &v) { viewport(v.l, v.b, v.w, v.h); }
  Viewport viewport() { return mViewportStack.get(); }
  void pushViewport();
  void popViewport();
  void pushViewport(int l, int b, int w, int h) {
    pushViewport();
    viewport(l, b, w, h);
  }
  void pushViewport(const Viewport &v) {
    pushViewport();
    viewport(v);
  }
  void pushViewport(int w, int h) {
    pushViewport();
    viewport(0, 0, w, h);
  }

  void framebuffer(EasyFBO &easyFBO) { framebuffer(easyFBO.fbo().id()); }
  void framebuffer(FBO &fbo) { framebuffer(fbo.id()); }
  void framebuffer(unsigned int id);
  // static unsigned int framebuffer() { return mFBOID; }
  unsigned int framebuffer() { return mFBOStack.get(); }
  void pushFramebuffer();
  void popFramebuffer();
  void pushFramebuffer(EasyFBO &f) {
    pushFramebuffer();
    framebuffer(f);
  }
  void pushFramebuffer(FBO &f) {
    pushFramebuffer();
    framebuffer(f);
  }
  void pushFramebuffer(unsigned int f) {
    pushFramebuffer();
    framebuffer(f);
  }

  void shader(ShaderProgram &s);
  ShaderProgram &shader() { return *mShaderPtr; }
  ShaderProgram *shaderPtr() { return mShaderPtr; }

  virtual void camera(Viewpoint const &v);
  virtual void camera(Viewpoint::SpecialType v);
  void pushCamera();
  void popCamera();
  void pushCamera(Viewpoint const &v) {
    pushCamera();
    camera(v);
  }
  void pushCamera(Viewpoint::SpecialType v) {
    pushCamera();
    camera(v);
  }

  virtual void update();
  void draw(VAOMesh &mesh);
  void draw(EasyVAO &vao);
  void draw(const Mesh &mesh);
  void draw(Mesh &&mesh);

protected:
  ShaderProgram *mShaderPtr = nullptr;
  std::unordered_map<unsigned int, int> modelviewLocs;
  std::unordered_map<unsigned int, int> projLocs;
  bool mShaderChanged = false;

  // let matrix stack be local to objects
  MatrixStack mViewStack;
  MatrixStack mProjStack;
  MatrixStack mModelStack;
  bool mMatChanged = false;

  ViewportStack mViewportStack;
  EasyVAO mInternalVAO;
  // unsigned int mFBOID = 0;
  FBOStack mFBOStack;
};

} // namespace al
#endif
