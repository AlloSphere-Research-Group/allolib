#include "al/graphics/al_DefaultShaderString.hpp"
#include <cstdlib>
#include <cstring>

// alloc only once with reserve
static std::string concatShaderStrings(const char** strings, int num) {
  size_t totalSize = 0;
  for (int i = 0; i < num; i += 1) {
    totalSize += std::strlen(strings[i]);
  }
  totalSize += 1;  // null terminator
  std::string result;
  result.reserve(totalSize);

  for (int i = 0; i < num; i += 1) {
    result += strings[i];
  }
  return result;
}

static const char* stereoFlat = R"(
vec4 stereoDisplace(vec4 v, float e, float f) {
    // eye to vertex distance
    float l = sqrt((v.x - e) * (v.x - e) + v.y * v.y + v.z * v.z);
    // absolute z-direction distance
    float z = abs(v.z);
    // x coord of projection of vertex on focal plane when looked from eye
    float t = f * (v.x - e) / z;
    // x of displaced vert to make displaced vert be projected on focal plane
    // when looked from origin at the same point orig vert would be projected
    // when looked form eye
    v.x = z * (e + t) / f;
    // set distance fromr origin to displaced vert same as eye to orig vertex
    v.xyz = normalize(v.xyz);
    v.xyz *= l;
    return v;
}

)";

static const char* stereoOmni = R"(
vec4 stereoDisplace(vec4 v, float e, float r) {
  vec3 OE = vec3(-v.z, 0.0, v.x); // eye dir, orthogonal to vert vec
  OE = normalize(OE);             // but preserving +y up-vector
  OE *= e;               // set mag to eye separation
  vec3 EV = v.xyz - OE;  // eye to vertex
  float ev = length(EV); // save length
  EV /= ev;              // normalize

  // coefs for polynomial t^2 + 2bt + c = 0
  // derived from cosine law r^2 = t^2 + e^2 + 2tecos(theta)
  // where theta is angle between OE and EV
  // t is distance to sphere surface from eye
  float b = -dot(OE, EV);         // multiply -1 to dot product cuz
                                  // OE needs to be flipped in dir
  float c = e * e - r * r;
  float t = -b + sqrt(b * b - c); // quadratic formula

  v.xyz = OE + t * EV;            // dir from orig to sphere surface
  v.xyz = ev * normalize(v.xyz);  // set mag to eye-to-v dist
  return v; 
}

)";

static const char* vertexShaderMatricesHeader() {
  return R"(
uniform mat4 alModelViewMatrix;
uniform mat4 alProjectionMatrix;
)";
}

static const char* vertexShaderStereoHeader(bool isStereo) {
  if (!isStereo) return "\n";
  return R"(
uniform float eyeOffset;
uniform float focalLength;
)";
}

static const char* vertexShaderLightingHeader(bool doLighting) {
  if (!doLighting) return "\n";
  return R"(
layout (location = 3) in vec3 vertexNormal;
uniform mat4 alNormalMatrix;
out vec3 normalEyeCoord;
out vec3 vertEyeCoord;
)";
}

static const char* vertexShaderCommonBody(bool isStereo) {
  if (isStereo) {
    return R"(
  vec4 ve = alModelViewMatrix * vec4(vertexPosition, 1.0);
  gl_Position = alProjectionMatrix
              * stereoDisplace(ve, eyeOffset, focalLength);
)";
  } else {
    return R"(
  vec4 ve = alModelViewMatrix * vec4(vertexPosition, 1.0);
  gl_Position = alProjectionMatrix * ve;
)";
  }
}

static const char* vertexShaderLightingBody(bool doLighting) {
  if (!doLighting) return "\n";
  // `ve` should be defined in previous function body (MV * p)
  return R"(
  normalEyeCoord = (alNormalMatrix * vec4(normalize(vertexNormal), 0.0)).xyz;
  vertEyeCoord = ve.xyz;
)";
}

static const char* fragmentShaderLightingHeader(bool doLighting) {
  if (!doLighting) return "\n";
  return R"(
uniform vec4 lightPositionEyeCoord;
in vec3 normalEyeCoord;
in vec3 vertEyeCoord;
)";
}

static const char* fragmentShaderLightingBody(bool doLighting) {
  if (!doLighting) return "\n";
  return R"(
  // w = 0 if directional light
  vec3 vertToLightEyeCoord = lightPositionEyeCoord.xyz
                           - vertEyeCoord * lightPositionEyeCoord.w;
  vec3 L = normalize(vertToLightEyeCoord);
  vec3 N = normalize(normalEyeCoord);
  float lambert = max(0.0, dot(N, L));
  c.rgb *= lambert;
)";
}

const char* al::shaderVersionString(int major, int minor) {
  if (major == 3 && minor == 3) return "#version 330\n";
  if (major == 4 && minor == 0) return "#version 400\n";
  if (major == 4 && minor == 1) return "#version 410\n";
  if (major == 4 && minor == 2) return "#version 420\n";
  if (major == 4 && minor == 3) return "#version 430\n";
  if (major == 4 && minor == 4) return "#version 440\n";
  if (major == 4 && minor == 5) return "#version 450\n";

  // fallback for invalid input
  return "#version 330\n";
}

const char* al::stereoVertexDisplaceFunctionString(bool isOmni) {
  if (isOmni) {
    return stereoOmni;
  } else {
    return stereoFlat;
  }
}

std::string al::vertexShaderStringP(int major, int minor, bool isStereo,
                                    bool isOmni, bool doLighting) {
  const char* sources[] = {
      al::shaderVersionString(major, minor),
      vertexShaderMatricesHeader(),
      R"(
layout (location = 0) in vec3 vertexPosition;
)",
      vertexShaderStereoHeader(isStereo),
      vertexShaderLightingHeader(doLighting),
      isStereo ? al::stereoVertexDisplaceFunctionString(isOmni) : "\n",
      R"(void main () {)",

      vertexShaderCommonBody(isStereo),
      vertexShaderLightingBody(doLighting),
      "}"};
  return concatShaderStrings(sources, 10);
}

std::string al::vertexShaderStringPC(int major, int minor, bool isStereo,
                                     bool isOmni, bool doLighting) {
  const char* sources[] = {
      al::shaderVersionString(major, minor),
      vertexShaderMatricesHeader(),
      R"(
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec4 vertexColor;
out vec4 vColor;
)",
      vertexShaderStereoHeader(isStereo),
      vertexShaderLightingHeader(doLighting),
      isStereo ? al::stereoVertexDisplaceFunctionString(isOmni) : "\n",
      R"(void main () {)",

      vertexShaderCommonBody(isStereo),
      vertexShaderLightingBody(doLighting),
      R"(
  vColor = vertexColor;
})"};
  return concatShaderStrings(sources, 10);
}

std::string al::vertexShaderStringPT(int major, int minor, bool isStereo,
                                     bool isOmni, bool doLighting) {
  const char* sources[] = {
      al::shaderVersionString(major, minor),
      vertexShaderMatricesHeader(),
      R"(
layout (location = 0) in vec3 vertexPosition;
layout (location = 2) in vec2 vertexTexcoord;
out vec2 vTexcoord;
)",
      vertexShaderStereoHeader(isStereo),
      vertexShaderLightingHeader(doLighting),
      isStereo ? al::stereoVertexDisplaceFunctionString(isOmni) : "\n",
      R"(
void main () {

)",
      vertexShaderCommonBody(isStereo),
      vertexShaderLightingBody(doLighting),
      R"(
  vTexcoord = vertexTexcoord;
})"};
  return concatShaderStrings(sources, 10);
}

std::string al::fragShaderStringU(int major, int minor, bool doLighting) {
  const char* sources[] = {al::shaderVersionString(major, minor),
                           fragmentShaderLightingHeader(doLighting),
                           R"(
uniform vec4 uColor;

layout (location = 0) out vec4 fragColor;

void main () {

  vec4 c = uColor;
)",
                           fragmentShaderLightingBody(doLighting),
                           R"(
  fragColor = c;
})"};
  return concatShaderStrings(sources, 5);
}

std::string al::fragShaderStringC(int major, int minor, bool doLighting) {
  const char* sources[] = {al::shaderVersionString(major, minor),
                           fragmentShaderLightingHeader(doLighting),
                           R"(
in vec4 vColor;

layout (location = 0) out vec4 fragColor;

void main () {
  vec4 c = vColor;
)",
                           fragmentShaderLightingBody(doLighting),
                           R"(
  fragColor = c;
})"};
  return concatShaderStrings(sources, 5);
}

std::string al::fragShaderStringT(int major, int minor, bool doLighting) {
  const char* sources[] = {al::shaderVersionString(major, minor),
                           fragmentShaderLightingHeader(doLighting),
                           R"(
uniform sampler2D tex0;
in vec2 vTexcoord;

layout (location = 0) out vec4 fragColor;

void main () {
  vec4 c = texture(tex0, vTexcoord);
)",
                           fragmentShaderLightingBody(doLighting),
                           R"(
  fragColor = c;
})"};
  return concatShaderStrings(sources, 5);
}

al::ShaderSources al::defaultShaderUniformColor(bool isStereo, bool isOmni,
                                                bool doLighting) {
  return {vertexShaderStringP(3, 3, isStereo, isOmni, doLighting),
          fragShaderStringU(3, 3, doLighting)};
}

al::ShaderSources al::defaultShaderVertexColor(bool isStereo, bool isOmni,
                                               bool doLighting) {
  return {vertexShaderStringPC(3, 3, isStereo, isOmni, doLighting),
          fragShaderStringC(3, 3, doLighting)};
}

al::ShaderSources al::defaultShaderTextureColor(bool isStereo, bool isOmni,
                                                bool doLighting) {
  return {vertexShaderStringPT(3, 3, isStereo, isOmni, doLighting),
          fragShaderStringT(3, 3, doLighting)};
}
