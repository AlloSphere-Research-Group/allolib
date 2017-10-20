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
    std::vector<Matrix4f> stack;

    MatrixStack() {
        // default constructor make identity matrix
        stack.emplace_back();
    }

    void push() {
        Matrix4f m = stack.back();
        stack.push_back(m);
    }

    void pop() {
        if (stack.size() > 1) { // why 1? don't pop all
            stack.pop_back();
        }
        else if (stack.size() == 1) {
            setIdentity();
        }
        else {
            stack.emplace_back();
        }
    }

    void mult(Matrix4f const& m) {
        stack.back() = stack.back() * m;
    }

    Matrix4f get() const {
        return stack.back();
    }

    void set(const Matrix4f& m) {
      stack.back() = m;
    }

    void setIdentity() {
        stack.back().setIdentity();
    }

};

class ViewportStack {
public:
    std::vector<Viewport> stack;

    ViewportStack() {
        stack.emplace_back();
    }

    void push() {
        Viewport v = stack.back();
        stack.push_back(v);
    }

    void pop() {
        if (stack.size() > 1) {
            stack.pop_back();
        }
        else if (stack.size() == 1) {
            return; // don't pop all
        }
        else { // no element, add one
            stack.emplace_back();
        }
    }

    Viewport get() const {
        return stack.back();
    }

    void set(const Viewport& m) {
      stack.back().set(m);
    }

    void set(int left, int bottom, int width, int height) {
      stack.back().set(left, bottom, width, height);
    }

};

class RenderManager {
public:
  
  void setClearColor(float r, float g, float b, float a = 1);
  void clearColor(int drawbuffer=0);
  void clearColor(float r, float g, float b, float a = 1, int drawbuffer = 0);
  void clearColor(Color const& c, int drawbuffer = 0);

  void setClearDepth(float d);
  void clearDepth();
  void clearDepth(float d);

  void clear(float r, float g, float b, float a=1, float d=1, int drawbuffer = 0);
  void clear(float grayscale, float d=1) { clear(grayscale, grayscale, grayscale, 1, d); }
  void clear(Color const& c, float d=1) { clear(c.r, c.g, c.b, c.a, d); }

  /// Push current matrix stack
  void pushMatrix();

  /// Pop current matrix stack
  void popMatrix();

  /// Set current matrix to identity
  void loadIdentity();

  /// Set current matrix
  //void loadMatrix(const Matrix4d &m) {/* !!!!!!!!!!!!! */}
  //void loadMatrix(const Matrix4f &m) {/* !!!!!!!!!!!!! */}

  /// Multiply current matrix
  //void multMatrix(const Matrix4d &m) {/* !!!!!!!!!!!!! */}
  //void multMatrix(const Matrix4f &m) {/* !!!!!!!!!!!!! */}

  Matrix4f modelMatrix() const { return mModelStack.get(); }
  Matrix4f viewMatrix() const { return mViewStack.get(); }
  Matrix4f projMatrix() const { return mProjStack.get(); }

  void modelMatrix(const Matrix4f& m);
  void viewMatrix(const Matrix4f& m);
  void projMatrix(const Matrix4f& m);

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

  /// Translate current matrix
  void translate(float x, float y, float z=0.);

  /// Translate current matrix
  template <class T>
  void translate(const Vec<3,T>& v){ translate(v[0],v[1],v[2]); }

  /// Translate current matrix
  template <class T>
  void translate(const Vec<2,T>& v){ translate(v[0],v[1]); }

  /// Set viewport
  void viewport(int left, int bottom, int width, int height);
  void viewport(int l, int b, int w, int h, float pixelDensity) {
    viewport(l * pixelDensity, b * pixelDensity, w * pixelDensity, h * pixelDensity);
  }
  void viewport(const Viewport& v) {
    viewport(v.l, v.b, v.w, v.h);
  }
  void viewport(const Viewport& v, float pixelDensity) {
    viewport(v.l, v.b, v.w, v.h, pixelDensity);
  }

  void scissor(int left, int bottom, int width, int height);

  void framebuffer(EasyFBO& easyFBO) { framebuffer(easyFBO.fbo().id()); }
  void framebuffer(FBO& fbo) { framebuffer(fbo.id()); }
  void framebuffer(unsigned int id);

  void shader(ShaderProgram& s);
  ShaderProgram& shader();
  ShaderProgram* shaderPtr() { return mShaderPtr; }

  void camera(Viewpoint const& v);
  void camera(Viewpoint const& v, int w, int h);
  void camera(Viewpoint const& v, int x, int y, int w, int h);
  void camera(Viewpoint::SpecialType v);
  void camera(Viewpoint::SpecialType v, int w, int h);
  void camera(Viewpoint::SpecialType v, int x, int y, int w, int h);

  void pushCamera();
  void popCamera();

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