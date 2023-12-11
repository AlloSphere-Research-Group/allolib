#ifndef INCLUDE_AL_GRAPHICS_DEFAULT_SHADER_STRING_HPP
#define INCLUDE_AL_GRAPHICS_DEFAULT_SHADER_STRING_HPP

#include <string>

namespace al {

struct ShaderSources {
  std::string vert;
  std::string frag;
};

// should be higher than 3.3
const char* shaderVersionString(int major, int minor);

// signature of the function is:
// `vec4 stereoDisplace (vec4 vertex, float eyeOffset, float focalLength)`
const char* stereoVertexDisplaceFunctionString(bool isOmni);

// P: position
// C: color
// T: texcoord
//
// layout (location = 0) in vec3 vertexPosition;
// layout (location = 1) in vec4 vertexColor;
// layout (location = 2) in vec2 vertexTexcoord;
// layout (location = 3) in vec3 vertexNormal;
// uniform mat4 alModelViewMatrix;
// uniform mat4 alProjectrionMatrix;
// uniform mat4 alNormalMatrix;
// uniform float eyeOffset;
// uniform float focalLength;
std::string vertexShaderStringP(int major, int minor, bool isStereo,
                                bool isOmni, bool doLighting);
std::string vertexShaderStringPC(int major, int minor, bool isStereo,
                                 bool isOmni, bool doLighting);
std::string vertexShaderStringPT(int major, int minor, bool isStereo,
                                 bool isOmni, bool doLighting);

// U: uniform color,
// C: per vertex color,
// T: per vertex texture sampling,
//
// uniform vec4 uColor;
// uniform sampler2D tex0;
// uniform vec4 lightPositionEyeCoord;
std::string fragShaderStringU(int major, int minor, bool doLighting);
std::string fragShaderStringC(int major, int minor, bool doLighting);
std::string fragShaderStringT(int major, int minor, bool doLighting);

// convenience functions
// assumes version 3.3
// example usage:
// auto src = defaultShaderUniformColor(true, false, 3);
// myShader.compile(src.vert.str, src.frag.str);
ShaderSources defaultShaderUniformColor(bool isStereo, bool isOmni,
                                        bool doLighting);
ShaderSources defaultShaderVertexColor(bool isStereo, bool isOmni,
                                       bool doLighting);
ShaderSources defaultShaderTextureColor(bool isStereo, bool isOmni,
                                        bool doLighting);

}  // namespace al

#endif
