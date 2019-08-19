#ifndef INCLUDE_AL_GRAPHICS_DEFAULT_SHADER_STRING_HPP
#define INCLUDE_AL_GRAPHICS_DEFAULT_SHADER_STRING_HPP

namespace al {

// A small memory managed c-string wrapper instead of std::string
// Does not keep user from doing memory operations (CAUTION!)
struct ShaderString {
  char* str = nullptr;
  ShaderString () = default;

  // alloc mem and cpy
  ShaderString (const ShaderString& other);

  // move ptr
  ShaderString (ShaderString&& other) noexcept;

  // del mem, alloc new mem and cpy
  ShaderString& operator= (const ShaderString& other);

  // swap ptr
  ShaderString& operator= (ShaderString&& other) noexcept;

  // del mem
  ~ShaderString ();
};

struct ShaderSources {
  ShaderString vert;
  ShaderString frag;
};

// should be higher than 3.3
const char* shaderVersionString (int major, int minor);

// signature of the function is:
// `vec4 stereoDisplace (vec4 vertex, float eyeOffset, float focalLength)`
const char* stereoVertexDisplaceFunctionString (bool isOmni);

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
ShaderString vertexShaderStringP (int major, int minor,
                                  bool isStereo, bool isOmni,
                                  bool doLighting);
ShaderString vertexShaderStringPC (int major, int minor,
                                   bool isStereo, bool isOmni,
                                   bool doLighting);
ShaderString vertexShaderStringPT (int major, int minor,
                                   bool isStereo, bool isOmni,
                                   bool doLighting);

// U: uniform color,
// C: per vertex color,
// T: per vertex texture sampling,
//
// uniform vec4 uColor;
// uniform sampler2D tex0;
// uniform vec4 lightPositionEyeCoord;
ShaderString fragShaderStringU (int major, int minor,
                                bool doLighting);
ShaderString fragShaderStringC (int major, int minor,
                                bool doLighting);
ShaderString fragShaderStringT (int major, int minor,
                                bool doLighting);

// conenience functions
// assumes version 3.3
// example usage:
// auto src = defaultShaderUniformColor(true, false, 3);
// myShader.compile(src.vert.str, src.frag.str);
ShaderSources defaultShaderUniformColor (bool isStereo, bool isOmni,
                                         bool doLighting);
ShaderSources defaultShaderVertexColor (bool isStereo, bool isOmni,
                                        bool doLighting);
ShaderSources defaultShaderTextureColor (bool isStereo, bool isOmni,
                                         bool doLighting);

} // namespace al

#endif

