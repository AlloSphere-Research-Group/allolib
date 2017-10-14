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

#include "al/core/math/al_Vec.hpp"
#include "al/core/math/al_Quat.hpp"
#include "al/core/math/al_Matrix4.hpp"
#include "al/core/types/al_Color.hpp"
#include "al/core/system/al_Printing.hpp"

#include "al/core/graphics/al_VAOMesh.hpp"
#include "al/core/graphics/al_EasyVAO.hpp"
#include "al/core/graphics/al_GLEW.hpp"
#include "al/core/graphics/al_Shader.hpp"
#include "al/core/graphics/al_Texture.hpp"
#include "al/core/graphics/al_Viewpoint.hpp"
#include "al/core/graphics/al_FBO.hpp"
#include "al/core/graphics/al_EasyFBO.hpp"
#include "al/core/graphics/al_OpenGL.hpp"
#include "al/core/io/al_Window.hpp"

#include <iostream>
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

    Matrix4f get() {
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

    Viewport get() {
        return stack.back();
    }

    void set(const Viewport& m) {
      stack.back().set(m);
    }

    void set(int left, int bottom, int width, int height) {
      stack.back().set(left, bottom, width, height);
    }

};

/// Interface for setting graphics state and rendering Mesh
/// @ingroup allocore
class Graphics {
public:

  Graphics(Window* window);

  /// Enable a capability
  void enable(Capability v){ glEnable(v); }

  /// Disable a capability
  void disable(Capability v){ glDisable(v); }

  /// Set a capability
  void capability(Capability cap, bool value);

  /// Turn blending on/off
  void blending(bool b);

  /// Turn color mask RGBA components on/off
  void colorMask(bool r, bool g, bool b, bool a);

  /// Turn color mask on/off (all RGBA components)
  void colorMask(bool b);

  /// Turn the depth mask on/off
  void depthMask(bool b);

  /// Turn depth testing on/off
  void depthTesting(bool b);

  /// Turn scissor testing on/off
  void scissorTest(bool b);

  /// Turn face culling on/off
  void cullFace(bool b);

  /// Turn face culling on/off and set the culled face
  void cullFace(bool b, Face face);


  /// Set both line width and point diameter
  // void stroke(float v){ lineWidth(v); pointSize(v); }

  /// Set width, in pixels, of lines
  // void lineWidth(float v);

  /// Set diameter, in pixels, of points
  void pointSize(float v);

  /// Set polygon drawing mode
  void polygonMode(PolygonMode m, Face f=FRONT_AND_BACK);

  /// Draw only edges of polygons with lines
  void polygonLine(Face f=FRONT_AND_BACK){ polygonMode(LINE,f); }

  /// Draw filled polygons
  void polygonFill(Face f=FRONT_AND_BACK){ polygonMode(FILL,f); }

  /// Set blend mode
  void blendMode(BlendFunc src, BlendFunc dst, BlendEq eq=FUNC_ADD);

  /// Set blend mode to additive (symmetric additive lighten)
  void blendModeAdd(){ blendMode(SRC_ALPHA, ONE, FUNC_ADD); }

  /// Set blend mode to subtractive (symmetric additive darken)
  void blendModeSub(){ blendMode(SRC_ALPHA, ONE, FUNC_REVERSE_SUBTRACT); }

  /// Set blend mode to screen (symmetric multiplicative lighten)
  void blendModeScreen(){ blendMode(ONE, ONE_MINUS_SRC_COLOR, FUNC_ADD); }

  /// Set blend mode to multiplicative (symmetric multiplicative darken)
  void blendModeMul(){ blendMode(DST_COLOR, ZERO, FUNC_ADD); }

  /// Set blend mode to transparent (asymmetric)
  void blendModeTrans(){ blendMode(SRC_ALPHA, ONE_MINUS_SRC_ALPHA, FUNC_ADD); }

  /// Turn blending states on (without setting mode)
  void blendOn(){ depthMask(false); blending(true); }

  /// Set states for additive blending
  void blendAdd(){ blendOn(); blendModeAdd(); }

  /// Set states for subtractive blending
  void blendSub(){ blendOn(); blendModeSub(); }

  /// Set states for screen blending
  void blendScreen(){ blendOn(); blendModeScreen(); }

  /// Set states for multiplicative blending
  void blendMul(){ blendOn(); blendModeMul(); }

  /// Set states for transparent blending
  void blendTrans(){ blendOn(); blendModeTrans(); }

  /// Turn blending states off (opaque rendering)
  void blendOff(){ depthMask(true); blending(false); }

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

  Matrix4f modelMatrix();
  Matrix4f viewMatrix();
  Matrix4f projMatrix();

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

  Window& window() { return *mWindowPtr; }

protected:
  Window* mWindowPtr;

  ShaderProgram* mShaderPtr = nullptr;
  bool mShaderChanged = false;
  std::unordered_map<unsigned int, int> modelviewLocs;
  std::unordered_map<unsigned int, int> projLocs;

  Matrix4f mViewMat;
  Matrix4f mProjMat;
  MatrixStack mViewStack;
  MatrixStack mProjStack;
  MatrixStack mModelStack;
  bool mMatChanged = false;

  Color mClearColor {0, 0, 0, 1};
  float mClearDepth = 1;

  EasyVAO mInternalVAO;

  unsigned int mFBOID = 0;

  Viewport mViewport;
  ViewportStack mViewportStack;
};

}
#endif
// al::
