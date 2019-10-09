#ifndef INCLUDE_AL_GRAPHICS_DEFAULT_SHADERS_HPP
#define INCLUDE_AL_GRAPHICS_DEFAULT_SHADERS_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
 */

/*

  Uniforms and Attributes in Vertex shader

      uniform mat4 al_ModelViewMatrix;
      uniform mat4 al_ProjectionMatrix;
      uniform mat4 al_NormalMatrix;

      layout (location = 0) in vec3 position;
      layout (location = 1) in vec4 color;
      layout (location = 2) in vec2 texcoord;
      layout (location = 3) in vec3 normal;

      uniform float eye_sep;
      uniform float foc_len;

      uniform vec4 light0_eye;
      uniform vec4 light1_eye;
      ...

  Uniforms in Fragment Shader

      uniform sampler2D tex0;
      uniform vec4 col0;
      uniform vec4 tint;
      uniform vec4 light_global_ambient;

      uniform vec4 light0_ambient;
      uniform vec4 light0_diffuse;
      uniform vec4 light0_specular;
      uniform float light0_enabled;
      ...

      uniform vec4 material_ambient;
      uniform vec4 material_diffuse;
      uniform vec4 material_specular;
      uniform float material_shininess;

*/

#include <string>
#include <vector>

#include "al/graphics/al_Shader.hpp"

inline constexpr int al_max_num_lights() { return 8; }

struct per_light_uniform_locations {
  int ambient = -1;
  int diffuse = -1;
  int specular = -1;
  int position = -1;
  int enabled = -1;
  int atten = -1;
};

struct material_uniform_locations {
  int ambient = -1;
  int diffuse = -1;
  int specular = -1;
  int shininess = -1;
  int emission = -1;
};

struct lighting_shader_uniforms {
  int global_ambient = -1;
  int normal_matrix = -1;
  int num_lights = 0;
  material_uniform_locations material;
  std::vector<per_light_uniform_locations> lights;
  bool has_material = false;
};

void al_print_lighting_uniforms(lighting_shader_uniforms const& u,
                                std::string name = "shader");

lighting_shader_uniforms al_get_lighting_uniform_locations(
    al::ShaderProgram& s, bool print_result = false);

inline std::string al_default_shader_version_string() {
  return R"(#version 330
)";
}

inline std::string al_default_vert_shader_stereo_functions(bool is_omni) {
  if (!is_omni)
    return R"(
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
  else
    return R"(
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

// ---------------------------------------------------------

inline std::string al_mesh_vert_shader(bool is_omni = false) {
  using namespace std::string_literals;
  return al_default_shader_version_string() +
         al_default_vert_shader_stereo_functions(is_omni) + R"(
uniform mat4 al_ModelViewMatrix;
uniform mat4 al_ProjectionMatrix;
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
uniform float eye_sep;
uniform float foc_len;
out vec4 color_;
void main() {
  if (eye_sep == 0) {
    gl_Position = al_ProjectionMatrix * al_ModelViewMatrix * vec4(position, 1.0);
  }
  else {
    gl_Position = al_ProjectionMatrix * stereo_displace(al_ModelViewMatrix * vec4(position, 1.0), eye_sep, foc_len);
  }
  color_ = color;
}
)"s;
}

inline std::string al_mesh_frag_shader() {
  return R"(
#version 330
uniform vec4 tint;
in vec4 color_;
out vec4 frag_color;
void main() {
  frag_color = color_ * tint;
}
)";
}

// ---------------------------------------------------------

inline std::string al_tex_vert_shader(bool is_omni = false) {
  using namespace std::string_literals;
  return al_default_shader_version_string() +
         al_default_vert_shader_stereo_functions(is_omni) + R"(
uniform mat4 al_ModelViewMatrix;
uniform mat4 al_ProjectionMatrix;
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texcoord;
uniform float eye_sep;
uniform float foc_len;
out vec2 texcoord_;
void main() {
  if (eye_sep == 0) {
    gl_Position = al_ProjectionMatrix * al_ModelViewMatrix * vec4(position, 1.0);
  }
  else {
    gl_Position = al_ProjectionMatrix * stereo_displace(al_ModelViewMatrix * vec4(position, 1.0), eye_sep, foc_len);
  }
  texcoord_ = texcoord;
}
)";
}

inline std::string al_tex_frag_shader() {
  return R"(
#version 330
uniform sampler2D tex0;
uniform vec4 tint;
in vec2 texcoord_;
out vec4 frag_color;
void main() {
  frag_color = texture(tex0, texcoord_) * tint;
}
)";
}

// ---------------------------------------------------------

inline std::string al_color_vert_shader(bool is_omni = false) {
  using namespace std::string_literals;
  return al_default_shader_version_string() +
         al_default_vert_shader_stereo_functions(is_omni) + R"(
uniform mat4 al_ModelViewMatrix;
uniform mat4 al_ProjectionMatrix;
layout (location = 0) in vec3 position;
uniform float eye_sep;
uniform float foc_len;
void main() {
  if (eye_sep == 0) {
    gl_Position = al_ProjectionMatrix * al_ModelViewMatrix * vec4(position, 1.0);
  }
  else {
    gl_Position = al_ProjectionMatrix * stereo_displace(al_ModelViewMatrix * vec4(position, 1.0), eye_sep, foc_len);
  }
}
)";
}

inline std::string al_color_frag_shader() {
  return R"(
#version 330
uniform vec4 col0;
uniform vec4 tint;
out vec4 frag_color;
void main() {
  frag_color = col0 * tint;
}
)";
}

// --------------------------------------------------------

namespace al {

enum class ShaderType : unsigned char {
  COLOR,             // uniform color
  MESH,              // per vertex mesh color
  TEXTURE,           // sample texture from texture coordinate per vertex
  LIGHTING_COLOR,    // ShaderType::COLOR with lighting
  LIGHTING_MESH,     // ShaderType::MESH with lighting
  LIGHTING_TEXTURE,  // ShaderType::TEXTURE with lighting
  LIGHTING_MATERIAL  // Use material for coloring with lighting
};

// compile shader program as with given config
void compileDefaultShader(ShaderProgram& s, ShaderType type,
                          bool is_omni = false);

// return vertex shader string for specified configuration
std::string multilight_vert_shader(ShaderType type, int num_lights,
                                   bool is_omni = false);

// return fragment shader string for specified configuration
std::string multilight_frag_shader(ShaderType type, int num_lights);

// compile shader program as with given config with lighting
// TODO - `al::compileDefaultShader`
//        overlaps functionality with this one
void compileMultiLightShader(ShaderProgram& s, ShaderType type, int num_lights,
                             bool is_omni = false);

}  // namespace al

#endif
