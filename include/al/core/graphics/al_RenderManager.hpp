#ifndef INCLUDE_AL_RENDER_MANAGER_HPP
#define INCLUDE_AL_RENDER_MANAGER_HPP

#include "al/core/graphics/al_EasyFBO.hpp"
#include "al/core/graphics/al_EasyVAO.hpp"
#include "al/core/graphics/al_FBO.hpp"
#include "al/core/graphics/al_Shader.hpp"
#include "al/core/graphics/al_VAOMesh.hpp"
#include "al/core/graphics/al_Viewpoint.hpp"
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
    void pop_all();
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

  void pushModelMatrix(const Matrix4f& m) { mModelStack.push(); modelMatrix(m); }
  void pushViewMatrix(const Matrix4f& m) { mViewStack.push(); viewMatrix(m); }
  void pushProjMatrix(const Matrix4f& m) { mProjStack.push(); projMatrix(m); }

  void popModelMatrix() { mModelStack.pop(); mMatChanged = true; }
  void popViewMatrix() { mViewStack.pop(); mMatChanged = true; }
  void popProjMatrix() { mProjStack.pop(); mMatChanged = true; }

  void resetModelMatrixStack() { mModelStack.pop_all(); mMatChanged = true; }
  void resetViewMatrixStack() { mViewStack.pop_all(); mMatChanged = true; }
  void resetProjMatrixStack() { mProjStack.pop_all(); mMatChanged = true; }
  void resetMatrixStack() {
    resetModelMatrixStack();
    resetViewMatrixStack();
    resetProjMatrixStack();
  }

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

  /// Scale current matrix along each dimension
  void scale(float x, float y, float z=1.);
  /// Scale current matrix uniformly
  void scale(float s) { scale(s, s, s); }
  /// Scale current matrix along each dimension
  template <class T>
  void scale(const Vec<3,T>& v){ scale(v[0],v[1],v[2]); }
  /// Scale current matrix along each dimension
  template <class T>
  void scale(const Vec<2,T>& v){ scale(v[0],v[1]); }


  /// Set viewport
  static void viewport(int left, int bottom, int width, int height);
  static void viewport(const Viewport& v) { viewport(v.l, v.b, v.w, v.h); }
  static Viewport viewport() { return mViewportStack.get(); }
  static void pushViewport();
  static void popViewport();
  static void pushViewport(int l, int b, int w, int h) {
    pushViewport(); viewport(l, b, w, h);
  }
  static void pushViewport(const Viewport& v) { pushViewport(); viewport(v); }
  static void pushViewport(int w, int h) {
    pushViewport(); viewport(0, 0, w, h);
  }

  static void framebuffer(EasyFBO& easyFBO) { framebuffer(easyFBO.fbo().id()); }
  static void framebuffer(FBO& fbo) { framebuffer(fbo.id()); }
  static void framebuffer(unsigned int id);
  // static unsigned int framebuffer() { return mFBOID; }
  static unsigned int framebuffer() { return mFBOStack.get(); }
  static void pushFramebuffer();
  static void popFramebuffer();
  static void pushFramebuffer(EasyFBO& f) { pushFramebuffer(); framebuffer(f);}
  static void pushFramebuffer(FBO& f) { pushFramebuffer(); framebuffer(f);}
  static void pushFramebuffer(unsigned int f) { pushFramebuffer(); framebuffer(f);}

  static void shader(ShaderProgram& s);
  static ShaderProgram& shader() { return *mShaderPtr; };
  static ShaderProgram* shaderPtr() { return mShaderPtr; }

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
  static ShaderProgram* mShaderPtr;
  static std::unordered_map<unsigned int, int> modelviewLocs;
  static std::unordered_map<unsigned int, int> projLocs;
  static bool mShaderChanged;

  // let matrix stack be local to objects
  MatrixStack mViewStack;
  MatrixStack mProjStack;
  MatrixStack mModelStack;
  static bool mMatChanged;

  static ViewportStack mViewportStack;
  static EasyVAO mInternalVAO;
  // static unsigned int mFBOID;
  static FBOStack mFBOStack;
};

}  // namespace al
#endif