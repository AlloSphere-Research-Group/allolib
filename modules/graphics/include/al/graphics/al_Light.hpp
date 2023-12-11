#ifndef INCLUDE_AL_LIGHT_HPP
#define INCLUDE_AL_LIGHT_HPP

#include "al/types/al_Color.hpp"

namespace al {

/**
@brief Light class
@ingroup Graphics
*/
struct Light {
  Light& pos(float x, float y, float z) {
    mPos[0] = x;
    mPos[1] = y;
    mPos[2] = z;
    mPos[3] = 1;
    return *this;
  }
  Light& dir(float x, float y, float z) {
    mPos[0] = x;
    mPos[1] = y;
    mPos[2] = z;
    mPos[3] = 0;
    return *this;
  }
  Light& ambient(const Color& v) {
    mAmbient = v;
    return *this;
  }
  Light& diffuse(const Color& v) {
    mDiffuse = v;
    return *this;
  }
  Light& specular(const Color& v) {
    mSpecular = v;
    return *this;
  }
  // Light& attenuation(float c0, float c1=0, float c2=0);

  const float* pos() const { return mPos; }
  // const float * dir() const { return mPos; }
  const Color& ambient() const { return mAmbient; }
  const Color& diffuse() const { return mDiffuse; }
  const Color& specular() const { return mSpecular; }
  // const float * attenuation() const { return mAtten; }

  static void globalAmbient(const Color& v) { mGlobalAmbient = v; }
  static const Color& globalAmbient() { return mGlobalAmbient; };

 private:
  // initial values are values from fixed pipeline defaults
  // https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glLight.xml
  Color mAmbient{0.0f};
  Color mDiffuse{1.0f};
  Color mSpecular{1.0f};
  float mPos[4]{0.0f, 0.0f, 1.0f, 0.0f};
  // float mAtten[4];

  static Color mGlobalAmbient;  // {0.2, 0.2, 0.2, 1.0}
};

/**
@brief Material class
@ingroup Graphics
*/
struct Material {
  Material& ambient(const Color& v) {
    mAmbient = v;
    return *this;
  }
  Material& diffuse(const Color& v) {
    mDiffuse = v;
    return *this;
  }
  // Material& emission(const Color& v) { mEmission = v; return *this; }
  Material& specular(const Color& v) {
    mSpecular = v;
    return *this;
  }
  Material& shininess(float v) {
    mShine = v;
    return *this;
  };  // [0, 128]

  const Color& ambient() const { return mAmbient; }
  const Color& diffuse() const { return mDiffuse; }
  // const Color& emission() const { return mEmission; }
  const Color& specular() const { return mSpecular; }
  float shininess() const { return mShine; }

 private:
  // initial values are values from fixed pipeline defaults
  // https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glMaterial.xml
  Color mAmbient{0.2f};
  Color mDiffuse{0.8f};
  // Color mEmission {0};
  Color mSpecular{0.0f};
  float mShine = 5.0f;
};

}  // namespace al

#endif