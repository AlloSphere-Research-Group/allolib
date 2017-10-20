#ifndef INCLUDE_AL_RENDER_MANAGER_HPP
#define INCLUDE_AL_RENDER_MANAGER_HPP

#include "al/core/graphics/al_EasyFBO.hpp"
#include "al/core/graphics/al_EasyVAO.hpp"
#include "al/core/graphics/al_FBO.hpp"
#include "al/core/graphics/al_Shader.hpp"
#include "al/core/graphics/al_VAOMesh.hpp"
#include "al/core/graphics/al_Viewpoint.hpp"
#include "al/core/io/al_Window.hpp"
#include "al/core/math/al_Matrix4.hpp"
#include "al/core/math/al_Quat.hpp"
#include "al/core/math/al_Vec.hpp"

#include <unordered_map>

namespace al {

class MatrixStack {
public:
    MatrixStack();
    void push();
    void pop();
    void mult(Matrix4f const& m);
    void set(const Matrix4f& m);
    void setIdentity();
    Matrix4f get() const;
private:
    std::vector<Matrix4f> stack;
};

class ViewportStack {
public:
    ViewportStack();
    void push();
    void pop();
    Viewport get() const;
    void set(const Viewport& m);
    void set(int left, int bottom, int width, int height);
private:
    std::vector<Viewport> stack;
};

class RenderManager {
public:

  /// Multiply current matrix
  void multModelMatrix(const Matrix4f &m) { mModelStack.mult(m); mMatChanged = true; }
  void multViewMatrix(const Matrix4f &m) { mViewStack.mult(m); mMatChanged = true;}
  void multProjMatrix(const Matrix4f &m) { mProjStack.mult(m); mMatChanged = true;}

  Matrix4f modelMatrix() const { return mModelStack.get(); }
  Matrix4f viewMatrix() const { return mViewStack.get(); }
  Matrix4f projMatrix() const { return mProjStack.get(); }

  void modelMatrix(const Matrix4f& m) { mModelStack.set(m); mMatChanged = true; }
  void viewMatrix(const Matrix4f& m) { mViewStack.set(m); mMatChanged = true; }
  void projMatrix(const Matrix4f& m) { mProjStack.set(m); mMatChanged = true; }

  void pushModelMatrix() { mModelStack.push(); }
  void pushViewMatrix() { mViewStack.push(); }
  void pushProjMatrix() { mProjStack.push(); }

  void popModelMatrix() { mModelStack.pop(); }
  void popViewMatrix() { mViewStack.pop(); }
  void popProjMatrix() { mProjStack.pop(); }

  /// Push current matrix stack
  void pushMatrix() { pushModelMatrix(); }

  /// Pop current matrix stack
  void popMatrix() { popModelMatrix(); }

  /// Set current matrix to identity
  void loadIdentity() { mModelStack.setIdentity(); }

  /// Translate current matrix
  void translate(float x, float y, float z=0.);
  /// Translate current matrix
  template <class T>
  void translate(const Vec<3,T>& v){ translate(v[0],v[1],v[2]); }
  /// Translate current matrix
  template <class T>
  void translate(const Vec<2,T>& v){ translate(v[0],v[1]); }

  /// Rotate current matrix
  /// \param[in] angle  angle, in degrees
  /// \param[in] x    x component of rotation axis
  /// \param[in] y    y component of rotation axis
  /// \param[in] z    z component of rotation axis
  void rotate(float angle, float x=0., float y=0., float z=1.);
  /// Rotate current matrix
  void rotate(const Quatf& q);
  /// Rotate current matrix
  /// \param[in] angle  angle, in degrees
  /// \param[in] axis   rotation axis
  template <class T>
  void rotate(float angle, const Vec<3,T>& axis){
    rotate(angle, axis[0],axis[1],axis[2]);
  }

  /// Scale current matrix uniformly
  void scale(float s);
  /// Scale current matrix along each dimension
  void scale(float x, float y, float z=1.);
  /// Scale current matrix along each dimension
  template <class T>
  void scale(const Vec<3,T>& v){ scale(v[0],v[1],v[2]); }
  /// Scale current matrix along each dimension
  template <class T>
  void scale(const Vec<2,T>& v){ scale(v[0],v[1]); }


  /// Set viewport
  void viewport(int left, int bottom, int width, int height);
  void viewport(const Viewport& v) { viewport(v.l, v.b, v.w, v.h); }
  Viewport viewport() { return mViewportStack.get(); }
  void pushViewport();
  void popViewport();
  void pushViewport(int l, int b, int w, int h) {
    pushViewport(); viewport(l, b, w, h);
  }
  void pushViewport(const Viewport& v) { pushViewport(); viewport(v); }

  void framebuffer(EasyFBO& easyFBO) { framebuffer(easyFBO.fbo().id()); }
  void framebuffer(FBO& fbo) { framebuffer(fbo.id()); }
  void framebuffer(unsigned int id);

  void shader(ShaderProgram& s);
  ShaderProgram& shader() { return *mShaderPtr; };
  ShaderProgram* shaderPtr() { return mShaderPtr; }

  void camera(Viewpoint const& v);
  void camera(Viewpoint::SpecialType v);
  void pushCamera();
  void popCamera();
  void pushCamera(Viewpoint const& v) { pushCamera(); camera(v); }
  void pushCamera(Viewpoint::SpecialType v) { pushCamera(); camera(v); }

  void update();
  void draw(VAOMesh& mesh);
  void draw(EasyVAO& vao);
  void draw(Mesh& mesh);
  void draw(Mesh&& mesh);

protected:
  ShaderProgram* mShaderPtr = nullptr;
  std::unordered_map<unsigned int, int> modelviewLocs;
  std::unordered_map<unsigned int, int> projLocs;
  bool mShaderChanged = false;

  MatrixStack mViewStack;
  MatrixStack mProjStack;
  MatrixStack mModelStack;
  bool mMatChanged = false;

  ViewportStack mViewportStack;

  EasyVAO mInternalVAO;

  unsigned int mFBOID = 0;
};

#if 0
class RenderManager {
 public:
 private:
  CameraStack mCameraStack;
  ViewportStack mViewportStack;
};

class RenderManagerBackEnd {
 public:
 private:
  Matrix4f mViewMat;
  Matrix4f mProjMat;
  Matrix4f mModelMat;

  ShaderProgram* mShaderPtr = nullptr;
  std::unordered_map<unsigned int, int> modelviewLocs;
  std::unordered_map<unsigned int, int> projLocs;

  bool mShaderChanged = false;
  bool mMatChanged = false;

  EasyVAO mInternalVAO;
};
#endif

}  // namespace al
#endif