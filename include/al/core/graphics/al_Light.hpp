#ifndef INCLUDE_AL_GRAPHICS_LIGHT_HPP
#define INCLUDE_AL_GRAPHICS_LIGHT_HPP

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
  Wrapper to OpenGL lighting & materials

  File author(s):
  Lance Putnam, 2010, putnam.lance@gmail.com
  Graham Wakefield, 2010, grrrwaaa@gmail.com
  Keehong Youn, 2017, younkeehong@gmail.com
*/

// #include <string>

#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/types/al_Color.hpp"

namespace al{

/// Material properties

/// These should be set before rendering the vertices of the object they
/// are to be applied to.
/// @ingroup allocore
class Material {
public:

  Material() {}

  Material& ambient(const Color& v) {}
  Material& diffuse(const Color& v) {}
  Material& specular(const Color& v) {}
  Material& shininess(float v) {}

  // Material& illumination(float v) { mIllumination=v; return *this; }
  // Material& opticalDensity(float v) { mOpticalDensity=v; return *this; }
  // Material& emission(const Color& v) {}

  const Color& ambient() const { return mAmbient; }
  const Color& diffuse() const  { return mDiffuse; }
  const Color& specular() const { return mSpecular; }
  float shininess() const { return mShine; }

  // float opticalDensity() const { return mOpticalDensity; }
  // float illumination() const { return mIllumination; }
  // const Color& emission() const { return mEmission; }

protected:
  Color mAmbient;
  Color mDiffuse;
  Color mSpecular;
  float mShine;
  // Color mEmission;
  // float mOpticalDensity, mIllumination;
};


/// Light
/// @ingroup allocore
class Light{
public:
  /// Attenuation factor = 1/(c0 + c1*d + c2*d*d)
  Light& attenuation(float c0, float c1=0, float c2=0) {
    mAtten[0]=c0; mAtten[1]=c1; mAtten[2]=c2; return *this;
  }

  Light& ambient(const Color& v) { mAmbient=v; return *this; }
  Light& diffuse(const Color& v) { mDiffuse=v; return *this; }
  Light& specular(const Color& v) { mSpecular=v; return *this; }
  Light& color(const Color& v) {
    mAmbient= v;
    mDiffuse= v;
    mSpecular= v;
    return *this;
  }
  
  /// Set directional light direction
  Light& dir(float x, float y, float z) {
    mDir[0]=x; mDir[1]=y; mDir[2]=z;
    return *this;
  }

  /// Set directional light direction
  template <class VEC3>
  Light& dir(const VEC3& v){ return dir(v[0],v[1],v[2]); }

  /// Set positional light position
  Light& pos(float x, float y, float z) {
    mPos[0]=x; mPos[1]=y; mPos[2]=z;
    return *this;
  }

  /// Set positional light position
  template <class VEC3>
  Light& pos(const VEC3& v){ return pos(v[0],v[1],v[2]); }

  /// Set spotlight parameters
  /// @param[in] cutoff  angle of the cone light emitted by the spot; [0, 90], 180 (uniform)
  /// @param[in] expo    the intensity distribution of the light; [0, 128]
  Light& spot(float cutoff, float exponent=1) {
    mSpotCutOff = cutoff;
    mSpotExponent = exponent;
    return *this;
  }

  /// Get position array
  const float * pos() const { return mPos; }
  float * pos(){ return mPos; }

  /// Get direction array
  const float * dir() const { return mDir; }
  float * dir(){ return mDir; }

  /// Get attenuation array
  const float * attenuation() const { return mAtten; }
  float * attenuation(){ return mAtten; }

  /// Get ambient color
  const Color& ambient() const { return mAmbient; }
  Color& ambient(){ return mAmbient; }

  /// Get diffuse color
  const Color& diffuse() const { return mDiffuse; }
  Color& diffuse(){ return mDiffuse; }

  /// Get specular color
  const Color& specular() const { return mSpecular; }
  Color& specular(){ return mSpecular; }

  /// Set global ambient light intensity (default is {0.2, 0.2, 0.2, 1})
  static void globalAmbient(const Color& v) { mGlobalAmbient = v; }

  /// Setting this to true effectively reverses normals of back-facing
  /// polygons.
  // static void twoSided(bool v);

protected:
  // default values: white point light at (0, 1000, 0)
  // with no global light
  
  float mIntensity = 1.0f; // also indicates enabled/disabled
  float mIsPointLight = 1.0f;

  Color mAmbient = Color{1.0f, 1.0f, 1.0f};
  Color mDiffuse = Color{1.0f, 1.0f, 1.0f};;
  Color mSpecular = Color{1.0f, 1.0f, 1.0f};;
  float mPos[4] = {0.0f, 1000.0f, 0.0f, 1.0f};
  float mDir[4] = {0.0f, -1.0f, 0.0f, 0,0f};
  float mAtten[3] = {1.0f, 0.0f, 0.0f};
  float mSpotCutOff = 180.0f;
  float mSpotExponent = 1.0f;

  static Color mGlobalAmbient = Color{0.0f, 0.0f, 0.0f};
};

} // ::al

#endif
