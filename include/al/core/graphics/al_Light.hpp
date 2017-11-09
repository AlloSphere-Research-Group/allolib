#ifndef INCLUDE_AL_LIGHT_HPP
#define INCLUDE_AL_LIGHT_HPP

namespace al {

struct Light
{
    Light& pos(float x, float y, float z) { mPos[0] = x; mPos[1] = y; mPos[2] = z; mPos[3] = 1; return *this; }
    Light& dir(float x, float y, float z) { mPos[0] = x; mPos[1] = y; mPos[2] = z; mPos[3] = 0; return *this; }
    Light& ambient(const Color& v) { mAmbient = v; return *this; }
    Light& diffuse(const Color& v) { mDiffuse = v; return *this; }
    // Light& specular(const Color& v) { mSpecular = v; return *this; }
    // Light& attenuation(float c0, float c1=0, float c2=0);

    const float * pos() const { return mPos; }
    // const float * dir() const { return mPos; }
    const Color& ambient() const { return mAmbient; }
    const Color& diffuse() const { return mDiffuse; }
    // const Color& specular() const { return mSpecular; }
    // const float * attenuation() const { return mAtten; }

    // static void globalAmbient(const Color& v);
    // static const Color& globalAmbient() const { return mGlobalAmbient; };

private:
    Color mAmbient;
    Color mDiffuse;
    // Color mSpecular;
    float mPos[4];
    // float mAtten[3];

    // static Color mGlobalAmbient;
};

struct Material
{
    // Material& ambient(const Color& v) { mAmbient = v; return *this; }
    Material& diffuse(const Color& v) { mDiffuse = v; return *this; }
    // Material& emission(const Color& v) { mEmission = v; return *this; }
    // Material& specular(const Color& v) { mSpecular = v; return *this; }
    // Material& shininess(float v) { mShine=v; return *this; }; // [0, 128]
    
    // const Color& ambient() const { return mAmbient; }
    const Color& diffuse() const  { return mDiffuse; }
    // const Color& emission() const { return mEmission; }
    // const Color& specular() const { return mSpecular; }
    // float shininess() const { return mShine; }

private:
    // Color mAmbient;
    Color mDiffuse;
    // Color mEmission;
    // Color mSpecular;
    // float mShine;
};

} // namespace al

#endif