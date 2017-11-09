#ifndef INCLUDE_AL_GRAPHICS_DEFAULT_SHADERS_HPP
#define INCLUDE_AL_GRAPHICS_DEFAULT_SHADERS_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/graphics/al_Shader.hpp"
#include <string>

inline std::string al_mesh_vert_shader() { return R"(
#version 330
uniform mat4 MV;
uniform mat4 P;
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
out vec4 color_;
void main() {
  gl_Position = P * MV * vec4(position, 1.0);
  color_ = color;
}
)";}

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

inline std::string al_tex_vert_shader() { return R"(
#version 330
uniform mat4 MV;
uniform mat4 P;
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texcoord;
out vec2 texcoord_;
void main() {
  gl_Position = P * MV * vec4(position, 1.0);
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

inline std::string al_color_vert_shader() { return R"(
#version 330
uniform mat4 MV;
uniform mat4 P;
layout (location = 0) in vec3 position;
void main() {
  gl_Position = P * MV * vec4(position, 1.0);
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
// out float light0_dist;
void main() {
  vec4 vert_eye = MV * vec4(position, 1.0);
  gl_Position = P * vert_eye;
  normal_eye = (N * vec4(normalize(normal), 0.0)).xyz;
  light0_dir = light0_eye.xyz - vert_eye.xyz * light0_eye.a;
  // light0_dist = length(light0_dir);
  light0_dir = normalize(light0_dir);
}
)";}

inline std::string al_lighting_color_frag_shader() { return R"(
#version 330
uniform vec4 col0;
uniform vec4 tint;
uniform vec4 light0_ambient;
uniform vec4 light0_diffuse;
in vec3 normal_eye;
in vec3 light0_dir;
// in float light0_dist;
out vec4 frag_color;
void main() {
  vec3 d0 = normalize(light0_dir);
  vec3 n = normalize(normal_eye);
  float nd0 = dot(d0, n);
  vec3 l0 = max(nd0, 0.0) * light0_diffuse.rgb + light0_ambient.rgb;
  frag_color = col0 * tint * vec4(l0, 1.0);
}
)";}

// ----------------------------------------------------------------------------

inline std::string al_lighting_mesh_vert_shader() { return R"(
#version 330
uniform mat4 MV;
uniform mat4 P;
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
out vec4 color_;
void main() {
  gl_Position = P * MV * vec4(position, 1.0);
  color_ = color;
}
)";}

inline std::string al_lighting_mesh_frag_shader() { return R"(
#version 330
uniform vec4 tint;
in vec4 color_;
out vec4 frag_color;
void main() {
  frag_color = color_ * tint;
}
)";}

// ----------------------------------------------------------------------------

inline std::string al_lighting_tex_vert_shader() { return R"(
#version 330
uniform mat4 MV;
uniform mat4 P;
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texcoord;
out vec2 texcoord_;
void main() {
  gl_Position = P * MV * vec4(position, 1.0);
  texcoord_ = texcoord;
}
)";}

inline std::string al_lighting_tex_frag_shader() { return R"(
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

inline std::string al_lighting_material_vert_shader() { return R"(
#version 330
uniform mat4 MV;
uniform mat4 P;
layout (location = 0) in vec3 position;
void main() {
  gl_Position = P * MV * vec4(position, 1.0);
}
)";}

inline std::string al_lighting_material_frag_shader() { return R"(
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

void compileDefaultShader(ShaderProgram& s, ShaderType type);

}

#endif
