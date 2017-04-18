#ifndef INCLUDE_AL_GRAPHICS_SHADER_HPP
#define INCLUDE_AL_GRAPHICS_SHADER_HPP

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
  Wrappers to OpenGL GLSL shaders

  File author(s):
  Lance Putnam, 2010, putnam.lance@gmail.com
  Graham Wakefield, 2010, grrrwaaa@gmail.com
  Wesley Smith, 2010, wesley.hoke@gmail.com
*/

#include <string>
#include <unordered_map>
#include "al/core/gl/al_GPUObject.hpp"
#include "al/core/math/al_Mat.hpp"
#include "al/core/math/al_Vec.hpp"
#include "al/core/types/al_Color.hpp"
#include "al/core/math/al_Quat.hpp"

#define AL_SHADER_MAX_LOG_SIZE  4096

namespace al{

inline std::string al_default_vert_shader() { return R"(
#version 330
uniform mat4 MVP;

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 normal;

out vec4 color_;
out vec2 texcoord_;
out vec3 normal_;

void main() {
  gl_Position = MVP * vec4(position, 1.0);
  color_ = color;
  texcoord_ = texcoord;
  normal_ = normal;
}
)";}

inline std::string al_default_frag_shader() { return R"(
#version 330
uniform sampler2D tex0;
uniform float tex0_mix;
uniform float light_mix;
uniform vec4 uniformColor;
uniform float uniformColorMix;
in vec4 color_;
in vec2 texcoord_;
in vec3 normal_;
out vec4 frag_color;
void main() {
  vec4 tex_color0 = texture(tex0, texcoord_);
  vec4 light_color = vec4(normal_, 1.0); // TODO
  vec4 color_val = mix(color_, uniformColor, uniformColorMix);
  vec4 final_color = mix(mix(color_val, tex_color0, tex0_mix), light_color, light_mix);
  frag_color = final_color;
}
)";}

/// Shader abstract base class
/// @ingroup allocore
class ShaderBase : public GPUObject{
public:

  virtual ~ShaderBase(){}

  /// Returns info log or 0 if none
  const char * log() const;

  /// Prints info log, if any
  void printLog() const;

protected:
  virtual void get(int pname, void * params) const = 0;
  virtual void getLog(char * buf) const = 0;
};



/// Shader object

/// A shader object represents your source code. You are able to pass your
/// source code to a shader object and compile the shader object.
/// @ingroup allocore
class Shader : public ShaderBase {
public:

  enum Type {
    VERTEX,
    GEOMETRY,
    FRAGMENT
  };

  Shader(const std::string& source="", Shader::Type type=FRAGMENT);

  /// This will automatically delete the shader object when it is no longer
  /// attached to any program object.
  virtual ~Shader(){ destroy(); }

  Shader& source(const std::string& v);
  Shader& source(const std::string& v, Shader::Type type);
  Shader& compile();
  bool compiled() const;

  Shader::Type type() const { return mType; }

private:
  std::string mSource;
  Shader::Type mType;
  void sendSource();

  virtual void get(int pname, void * params) const;
  virtual void getLog(char * buf) const;

  virtual void onCreate();
  virtual void onDestroy();
};



/// Shader program object

/// A program object represents a useable part of render pipeline.
/// It links one or more shader units into a single program object.
/// @ingroup allocore
class ShaderProgram : public ShaderBase {
public:

  /*!
    The basic parameter types
  */
  enum Type {
    NONE = 0,  //uninitialized type

    FLOAT,    ///< A single float value
    VEC2,    ///< Two float values
    VEC3,    ///< Three float values
    VEC4,    ///< Four float values

    INT,    ///< A single int value
    INT2,    ///< Two int values
    INT3,    ///< Three int values
    INT4,    ///< Four int values

    BOOL,    ///< A single bool value
    BOOL2,    ///< Two bool values
    BOOL3,    ///< Three bool values
    BOOL4,    ///< Four bool values

    MAT22,    ///< A 2x2 matrix
    MAT33,    ///< A 3x3 matrix
    MAT44,    ///< A 4x4 matrix

    SAMPLER_1D,      ///< A 1D texture
    SAMPLER_2D,      ///< A 2D texture
    SAMPLER_RECT,    ///< A rectangular texture
    SAMPLER_3D,      ///< A 3D texture
    SAMPLER_CUBE,    ///< A cubemap texture
    SAMPLER_1D_SHADOW,  ///< A 1D depth texture
    SAMPLER_2D_SHADOW  ///< A 2D depth texture

    //textures? non square matrices? attributes?
  };

  struct Attribute {
  };


  ShaderProgram();

  /// Any attached shaders will automatically be detached, but not deleted.
  virtual ~ShaderProgram();

  /// Attach shader to program

  /// The input shader will be compiled if necessary.
  ///
  ShaderProgram& attach(Shader& s);

  /// Detach shader from program
  const ShaderProgram& detach(const Shader& s) const;

  /// Link attached shaders

  /// @param[in] doValidate  validate program after linking;
  ///    You might not want to do this if you need to set uniforms before
  ///    validating, e.g., when using different texture sampler types in the
  ///    same shader.
  const ShaderProgram& link(bool doValidate=true) const;


  /// Compile and link shader sources
  bool compile(
    const std::string& vertSource,
    const std::string& fragSource,
    const std::string& geomSource=""
  );

  const ShaderProgram& use();

  /// Get whether program is active
  bool active() const { return mActive; }

  /// Set whether program is active
  ShaderProgram& active(bool v){ mActive=v; return *this; }

  /// Toggle active state
  ShaderProgram& toggleActive(){ mActive^=true; return *this; }

  /// Begin use of shader program
  bool begin();

  /// End use of shader program
  void end() const;


  /// Returns whether program linked successfully
  bool linked() const;

  /// Returns whether linked program can execute in current graphics state
  bool validateProgram(bool printLog=false) const;


  // These parameters must be set before attaching geometry shaders
  // void setGeometryInputPrimitive(Graphics::Primitive prim){ mInPrim = prim; }
  // void setGeometryOutputPrimitive(Graphics::Primitive prim){ mOutPrim = prim; }
  // void setGeometryOutputVertices(unsigned int i){ mOutVertices = i; }

  /// Print out all the input parameters to the shader
  void listParams() const;

  /// Get location of uniform
  int uniform(const char * name, bool warn = true) const;

  /// Get location of attribute
  int attribute(const char * name) const;

  const ShaderProgram& uniform(const char * name, Color v, bool warn = true) const {
    return uniform(name, v[0], v[1], v[2], v[3], warn);
  }

  const ShaderProgram& uniform(int loc, int v) const;
  const ShaderProgram& uniform(int loc, float v) const;
  const ShaderProgram& uniform(int loc, double v) const { return uniform(loc, float(v)); }
  const ShaderProgram& uniform(int loc, float v0, float v1) const;
  const ShaderProgram& uniform(int loc, float v0, float v1, float v2) const;
  const ShaderProgram& uniform(int loc, float v0, float v1, float v2, float v3) const;
  template <typename T>
  const ShaderProgram& uniform(int loc, const Vec<2,T>& v) const {
    return uniform(loc, v.x, v.y);
  }
  template <typename T>
  const ShaderProgram& uniform(int loc, const Vec<3,T>& v) const {
    return uniform(loc, v.x, v.y, v.z);
  }
  template <typename T>
  const ShaderProgram& uniform(int loc, const Vec<4,T>& v) const {
    return uniform(loc, v.x, v.y, v.z, v.w);
  }
  const ShaderProgram& uniformMatrix3(int loc, const float * v, bool transpose=false) const;
  const ShaderProgram& uniformMatrix4(int loc, const float * v, bool transpose=false) const;
  const ShaderProgram& uniform(int loc, const Mat<4,float>& m) const{
    return uniformMatrix4(loc, m.elems());
  }
  template<typename T>
  const ShaderProgram& uniform(int loc, const Mat<4,T>& m) const{
    return uniform(loc, Mat4f(m));
  }

  const ShaderProgram& uniform(const char * name, int v, bool warn = true) const;
  const ShaderProgram& uniform(const char * name, float v, bool warn = true) const;
  const ShaderProgram& uniform(const char * name, double v, bool warn = true) const { return uniform(name, float(v)); }
  const ShaderProgram& uniform(const char * name, float v0, float v1, bool warn = true) const;
  const ShaderProgram& uniform(const char * name, float v0, float v1, float v2, bool warn = true) const;
  const ShaderProgram& uniform(const char * name, float v0, float v1, float v2, float v3, bool warn = true) const;

  template <typename T>
  const ShaderProgram& uniform(const char * name, const Vec<2,T>& v, bool warn = true) const {
    return uniform(name, v.x, v.y, warn);
  }

  template <typename T>
  const ShaderProgram& uniform(const char * name, const Vec<3,T>& v, bool warn = true) const {
    return uniform(name, v.x, v.y, v.z, warn);
  }

  template <typename T>
  const ShaderProgram& uniform(const char * name, const Vec<4,T>& v, bool warn = true) const {
    return uniform(name, v.x, v.y, v.z, v.w, warn);
  }

  const ShaderProgram& uniform(const char * name, const Mat<4,float>& m, bool warn = true, bool transpose=false) const{
    return uniformMatrix4(name, m.elems(), transpose, warn);
  }

  template<typename T>
  const ShaderProgram& uniform(const char * name, const Mat<4,T>& m, bool warn = true, bool transpose=false) const{
    return uniform(name, Mat4f(m), transpose, warn);
  }

  template <typename T>
  const ShaderProgram& uniform(const char * name, const Quat<T>& q, bool warn = true) const {
    // note wxyz => xyzw for GLSL vec4:
    return uniform(name, q.x, q.y, q.z, q.w, warn);
  }


  const ShaderProgram& uniform1(const char * name, const float * v, int count=1, bool warn = true) const;
  const ShaderProgram& uniform2(const char * name, const float * v, int count=1, bool warn = true) const;
  const ShaderProgram& uniform3(const char * name, const float * v, int count=1, bool warn = true) const;
  const ShaderProgram& uniform4(const char * name, const float * v, int count=1, bool warn = true) const;

  const ShaderProgram& uniformMatrix3(const char * name, const float * v, bool transpose=false, bool warn = true) const;
  const ShaderProgram& uniformMatrix4(const char * name, const float * v, bool transpose=false, bool warn = true) const;


  const ShaderProgram& attribute(int loc, float v) const;
  const ShaderProgram& attribute(int loc, float v0, float v1) const;
  const ShaderProgram& attribute(int loc, float v0, float v1, float v2) const;
  const ShaderProgram& attribute(int loc, float v0, float v1, float v2, float v3) const;

  const ShaderProgram& attribute(const char * name, float v) const;
  const ShaderProgram& attribute(const char * name, float v0, float v1) const;
  const ShaderProgram& attribute(const char * name, float v0, float v1, float v2) const;
  const ShaderProgram& attribute(const char * name, float v0, float v1, float v2, float v3) const;

  const ShaderProgram& attribute1(const char * name, const float * v) const;
  const ShaderProgram& attribute2(const char * name, const float * v) const;
  const ShaderProgram& attribute3(const char * name, const float * v) const;
  const ShaderProgram& attribute4(const char * name, const float * v) const;
  const ShaderProgram& attribute1(int loc, const double * v) const;
  const ShaderProgram& attribute2(int loc, const double * v) const;
  const ShaderProgram& attribute3(int loc, const double * v) const;
  const ShaderProgram& attribute4(int loc, const double * v) const;

  template<typename T>
  const ShaderProgram& attribute(int loc, const Vec<2,T>& v) const {
    return attribute(loc, v.x, v.y);
  }
  template<typename T>
  const ShaderProgram& attribute(int loc, const Vec<3,T>& v) const {
    return attribute(loc, v.x, v.y, v.z);
  }
  template<typename T>
  const ShaderProgram& attribute(int loc, const Vec<4,T>& v) const {
    return attribute(loc, v.x, v.y, v.z, v.w);
  }
  template<typename T>
  const ShaderProgram& attribute(int loc, const Quat<T>& q) const {
    // note wxyz => xyzw for GLSL vec4:
    return attribute(loc, q.x, q.y, q.z, q.w);
  }



  static void use(unsigned programID);

protected:
  // Graphics::Primitive mInPrim, mOutPrim;  // IO primitives for geometry shaders
  // unsigned int mOutVertices;
  std::string mVertSource, mFragSource, mGeomSource;
  mutable std::unordered_map<std::string, int> mUniformLocs, mAttribLocs;
  bool mActive;

  virtual void get(int pname, void * params) const;
  virtual void getLog(char * buf) const;

  virtual void onCreate();
  virtual void onDestroy();
};

} // ::al

#endif
