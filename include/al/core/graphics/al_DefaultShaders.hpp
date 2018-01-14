#ifndef INCLUDE_AL_GRAPHICS_DEFAULT_SHADERS_HPP
#define INCLUDE_AL_GRAPHICS_DEFAULT_SHADERS_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/graphics/al_Shader.hpp"
#include <string>
#include <vector>

inline constexpr int al_max_num_lights() { return 8; }

struct per_light_uniform_locations
{
    int ambient = -1;
    int diffuse = -1;
    int specular = -1;
    int position = -1;
    int enabled = -1;
    int atten = -1;
};

struct material_uniform_locations
{
    int ambient = -1;
    int diffuse = -1;
    int specular = -1;
    int shininess = -1;
    int emission = -1;
};

struct lighting_shader_uniforms
{
    int global_ambient = -1;
    int normal_matrix = -1;
    int num_lights = 0;
    material_uniform_locations material;
    std::vector<per_light_uniform_locations> lights;
    bool has_material = false;
};

void al_print_lighting_uniforms(lighting_shader_uniforms const& u, std::string name = "shader");

lighting_shader_uniforms al_get_lighting_uniform_locations(al::ShaderProgram& s,
                                                           bool print_result=false);

inline std::string al_default_shader_version_string() {
  return R"(#version 330
)";
}

inline std::string al_default_vert_shader_stereo_functions(bool is_omni) {
  if (!is_omni) return R"(
vec4 stereo_displace(vec4 v, float e, float f) {
    // eye to vertex distance
    float l = sqrt((v.x - e) * (v.x - e) + v.y * v.y + v.z * v.z);
    // absolute z-direction distance
    float z = abs(v.z);
    // x coord of projection of vertex on focal plane when looked from eye
    float t = f * (v.x - e) / z;
    // x coord of displaced vertex to make displaced vertex be projected on focal plane
    // when looked from origin at the same point original vertex would be projected
    // when looked form eye
    v.x = z * (e + t) / f;
    // set distance fromr origin to displaced vertex same as eye to original vertex
    v.xyz = normalize(v.xyz);
    v.xyz *= l;
    return v;
})";
  else return R"(
vec4 stereo_displace(vec4 v, float e, float r) {
    vec3 OE = vec3(-v.z, 0.0, v.x); // eye direction, orthogonal to vertex vector
    OE = normalize(OE);             // but preserving +y up-vector
    OE *= e;               // set mag to eye separation
    vec3 EV = v.xyz - OE;  // eye to vertex
    float ev = length(EV); // save length
    EV /= ev;              // normalize

    // coefs for polynomial t^2 + 2bt + c = 0
    // derived from cosine law r^2 = t^2 + e^2 + 2tecos(theta)
    // where theta is angle between OE and EV
    // t is distance to sphere surface from eye
    float b = -dot(OE, EV);         // multiply -1 to dot product because
                                    // OE needs to be flipped in direction
    float c = e * e - r * r;
    float t = -b + sqrt(b * b - c); // quadratic formula

    v.xyz = OE + t * EV;            // direction from origin to sphere surface
    v.xyz = ev * normalize(v.xyz);  // normalize and set mag to eye-to-v distance
    return v; 
})";
}

// ----------------------------------------------------------------------------

inline std::string al_mesh_vert_shader(bool is_omni=false) {
  using namespace std::string_literals;
  return al_default_shader_version_string()
  + al_default_vert_shader_stereo_functions(is_omni)
  + R"(
uniform mat4 MV;
uniform mat4 P;
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
uniform float eye_sep;
uniform float foc_len;
out vec4 color_;
void main() {
  // gl_Position = P * MV * vec4(position, 1.0);
  gl_Position = P * stereo_displace(MV * vec4(position, 1.0), eye_sep, foc_len);
  color_ = color;
}
)"s;}

inline std::string al_mesh_frag_shader() { return R"(
#version 330
uniform vec4 tint;
in vec4 color_;
out vec4 frag_color;
void main() {
  frag_color = color_ * tint;
}
)";}

// ----------------------------------------------------------------------------

inline std::string al_tex_vert_shader(bool is_omni=false) {
  using namespace std::string_literals;
  return al_default_shader_version_string()
  + al_default_vert_shader_stereo_functions(is_omni)
  + R"(
uniform mat4 MV;
uniform mat4 P;
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texcoord;
uniform float eye_sep;
uniform float foc_len;
out vec2 texcoord_;
void main() {
  // gl_Position = P * MV * vec4(position, 1.0);
  gl_Position = P * stereo_displace(MV * vec4(position, 1.0), eye_sep, foc_len);
  texcoord_ = texcoord;
}
)";}

inline std::string al_tex_frag_shader() { return R"(
#version 330
uniform sampler2D tex0;
uniform vec4 tint;
in vec2 texcoord_;
out vec4 frag_color;
void main() {
  frag_color = texture(tex0, texcoord_) * tint;
}
)";}

// ----------------------------------------------------------------------------

inline std::string al_color_vert_shader(bool is_omni=false) {
  using namespace std::string_literals;
  return al_default_shader_version_string()
  + al_default_vert_shader_stereo_functions(is_omni)
  + R"(
uniform mat4 MV;
uniform mat4 P;
layout (location = 0) in vec3 position;
uniform float eye_sep;
uniform float foc_len;
void main() {
  // gl_Position = P * MV * vec4(position, 1.0);
  gl_Position = P * stereo_displace(MV * vec4(position, 1.0), eye_sep, foc_len);
}
)";}

inline std::string al_color_frag_shader() { return R"(
#version 330
uniform vec4 col0;
uniform vec4 tint;
out vec4 frag_color;
void main() {
  frag_color = col0 * tint;
}
)";}

// ----------------------------------------------------------------------------

namespace al {

enum class ShaderType : unsigned char {
  COLOR,
  MESH,
  TEXTURE,
  LIGHTING_COLOR,
  LIGHTING_MESH,
  LIGHTING_TEXTURE,
  LIGHTING_MATERIAL
};

void compileDefaultShader(ShaderProgram& s, ShaderType type, bool is_omni=false);

std::string multilight_vert_shader(ShaderType type, int num_lights, bool is_omni=false);
std::string multilight_frag_shader(ShaderType type, int num_lights);
void compileMultiLightShader(ShaderProgram& s, ShaderType type, int num_lights, bool is_omni=false);

}

#endif


#if 0

// ----------------------------------------------------------------------------

inline std::string al_lighting_color_vert_shader() { return R"(
#version 330
uniform mat4 MV;
uniform mat4 P;
uniform mat4 N; // normal matrix: transpose of inverse of MV
uniform vec4 light0_eye; // if fourth component is 1 point light, 0, directional
layout (location = 0) in vec3 position;
layout (location = 3) in vec3 normal;
out vec3 normal_eye;
out vec3 light0_dir;
out vec3 eye_dir;
// out float light0_dist;
void main() {
  vec4 vert_eye = MV * vec4(position, 1.0);
  gl_Position = P * vert_eye;
  normal_eye = (N * vec4(normalize(normal), 0.0)).xyz;
  light0_dir = light0_eye.xyz - vert_eye.xyz * light0_eye.a;
  // light0_dist = length(light0_dir);
  eye_dir = -vert_eye.xyz;
}
)";}

inline std::string al_lighting_color_frag_shader() { return R"(
#version 330
uniform vec4 col0;
uniform vec4 tint;
uniform vec4 light_global_ambient;
uniform vec4 light0_ambient;
uniform vec4 light0_diffuse;
uniform vec4 light0_specular;
uniform float light0_enabled;
in vec3 normal_eye;
in vec3 light0_dir;
in vec3 eye_dir;
// in float light0_dist;
out vec4 frag_color;
void main() {
  vec3 d0 = normalize(light0_dir); // to light
  vec3 n = normalize(normal_eye); // normal
  vec3 e = normalize(eye_dir); // to eye
  vec3 r = -reflect(d0, n); // reflection vector
  float n_d0 = max(dot(d0, n), 0.0);
  float e_r = max(dot(e, r), 0.0);
  // shininess 5.0 is OpenGL 2.x default value
  vec3 l0 = light0_ambient.rgb + n_d0 * light0_diffuse.rgb + light0_specular.rgb * pow(e_r, 5.0);
  l0 *= light0_enabled;
  l0 += light_global_ambient.rgb;
  frag_color = col0 * tint * vec4(l0, 1.0);
}
)";}

// ----------------------------------------------------------------------------

inline std::string al_lighting_mesh_vert_shader() { return R"(
#version 330
uniform mat4 MV;
uniform mat4 P;
uniform mat4 N; // normal matrix: transpose of inverse of MV
uniform vec4 light0_eye; // if fourth component is 1 point light, 0, directional
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 3) in vec3 normal;
out vec4 color_;
out vec3 normal_eye;
out vec3 light0_dir;
out vec3 eye_dir;
// out float light0_dist;
void main() {
  vec4 vert_eye = MV * vec4(position, 1.0);
  gl_Position = P * vert_eye;
  normal_eye = (N * vec4(normalize(normal), 0.0)).xyz;
  light0_dir = light0_eye.xyz - vert_eye.xyz * light0_eye.a;
  // light0_dist = length(light0_dir);
  eye_dir = -vert_eye.xyz;
  color_ = color;
}
)";}

inline std::string al_lighting_mesh_frag_shader() { return R"(
#version 330
uniform vec4 tint;
uniform vec4 light_global_ambient;
uniform vec4 light0_ambient;
uniform vec4 light0_diffuse;
uniform vec4 light0_specular;
uniform float light0_enabled;
in vec4 color_;
in vec3 normal_eye;
in vec3 light0_dir;
in vec3 eye_dir;
// in float light0_dist;
out vec4 frag_color;
void main() {
  vec3 d0 = normalize(light0_dir); // to light
  vec3 n = normalize(normal_eye); // normal
  vec3 e = normalize(eye_dir); // to eye
  vec3 r = -reflect(d0, n); // reflection vector
  float n_d0 = max(dot(d0, n), 0.0);
  float e_r = max(dot(e, r), 0.0);
  // shininess 5.0 is OpenGL 2.x default value
  vec3 l0 = light0_ambient.rgb + n_d0 * light0_diffuse.rgb + light0_specular.rgb * pow(e_r, 5.0);
  l0 *= light0_enabled;
  l0 += light_global_ambient.rgb;
  frag_color = color_ * tint * vec4(l0, 1.0);
}
)";}

// ----------------------------------------------------------------------------

inline std::string al_lighting_tex_vert_shader() { return R"(
#version 330
uniform mat4 MV;
uniform mat4 P;
uniform mat4 N; // normal matrix: transpose of inverse of MV
uniform vec4 light0_eye; // if fourth component is 1 point light, 0, directional
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 normal;
out vec2 texcoord_;
out vec3 normal_eye;
out vec3 light0_dir;
out vec3 eye_dir;
// out float light0_dist;
void main() {
  vec4 vert_eye = MV * vec4(position, 1.0);
  gl_Position = P * vert_eye;
  normal_eye = (N * vec4(normalize(normal), 0.0)).xyz;
  light0_dir = light0_eye.xyz - vert_eye.xyz * light0_eye.a;
  // light0_dist = length(light0_dir);
  eye_dir = -vert_eye.xyz;
  texcoord_ = texcoord;
}
)";}

inline std::string al_lighting_tex_frag_shader() { return R"(
#version 330
uniform sampler2D tex0;
uniform vec4 tint;
uniform vec4 light_global_ambient;
uniform vec4 light0_ambient;
uniform vec4 light0_diffuse;
uniform vec4 light0_specular;
uniform float light0_enabled;
in vec2 texcoord_;
in vec3 normal_eye;
in vec3 light0_dir;
in vec3 eye_dir;
// in float light0_dist;
out vec4 frag_color;
void main() {
  vec3 d0 = normalize(light0_dir); // to light
  vec3 n = normalize(normal_eye); // normal
  vec3 e = normalize(eye_dir); // to eye
  vec3 r = -reflect(d0, n); // reflection vector
  float n_d0 = max(dot(d0, n), 0.0);
  float e_r = max(dot(e, r), 0.0);
  // shininess 5.0 is OpenGL 2.x default value
  vec3 l0 = light0_ambient.rgb + n_d0 * light0_diffuse.rgb + light0_specular.rgb * pow(e_r, 5.0);
  l0 *= light0_enabled;
  l0 += light_global_ambient.rgb;
  frag_color = texture(tex0, texcoord_) * tint * vec4(l0, 1.0);
}
)";}

// ----------------------------------------------------------------------------

inline std::string al_lighting_material_vert_shader() { return R"(
#version 330
uniform mat4 MV;
uniform mat4 P;
uniform mat4 N; // normal matrix: transpose of inverse of MV
uniform vec4 light0_eye; // if fourth component is 1 point light, 0, directional
layout (location = 0) in vec3 position;
layout (location = 3) in vec3 normal;
out vec3 normal_eye;
out vec3 light0_dir;
out vec3 eye_dir;
// out float light0_dist;
void main() {
  vec4 vert_eye = MV * vec4(position, 1.0);
  gl_Position = P * vert_eye;
  normal_eye = (N * vec4(normalize(normal), 0.0)).xyz;
  light0_dir = light0_eye.xyz - vert_eye.xyz * light0_eye.a;
  // light0_dist = length(light0_dir);
  eye_dir = -vert_eye.xyz;
}
)";}

inline std::string al_lighting_material_frag_shader() { return R"(
#version 330
uniform vec4 tint;
uniform vec4 light_global_ambient;
uniform vec4 light0_ambient;
uniform vec4 light0_diffuse;
uniform vec4 light0_specular;
uniform float light0_enabled;
uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;
uniform float material_shininess;
in vec3 normal_eye;
in vec3 light0_dir;
in vec3 eye_dir;
// in float light0_dist;
out vec4 frag_color;
void main() {
  vec3 d0 = normalize(light0_dir); // to light
  vec3 n = normalize(normal_eye); // normal
  vec3 e = normalize(eye_dir); // to eye
  vec3 r = -reflect(d0, n); // reflection vector
  float n_d0 = max(dot(d0, n), 0.0);
  float e_r = max(dot(e, r), 0.0);
  // shininess 5.0 is OpenGL 2.x default value
  vec3 l0 = material_ambient.rgb  * light0_ambient.rgb
          + material_diffuse.rgb  * light0_diffuse.rgb  * n_d0
          + material_specular.rgb * light0_specular.rgb * pow(e_r, material_shininess);
  l0 *= light0_enabled;
  l0 += material_ambient.rgb * light_global_ambient.rgb;
  frag_color = tint * vec4(l0, 1.0);
}
)";}
#endif