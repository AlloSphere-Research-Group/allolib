#ifndef INCLUDE_AL_GRAPHICS_DEFAULT_SHADERS_HPP
#define INCLUDE_AL_GRAPHICS_DEFAULT_SHADERS_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
*/

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

in vec4 color_;

out vec4 frag_color;

void main() {
  frag_color = color_;
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

in vec2 texcoord_;

out vec4 frag_color;

void main() {
  frag_color = texture(tex0, texcoord_);
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

out vec4 frag_color;

void main() {
  frag_color = col0;
}
)";}

// ----------------------------------------------------------------------------

inline std::string al_default_vert_shader() { return R"(
#version 330
uniform mat4 MV;
uniform mat4 P;

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texcoord;

out vec4 color_;
out vec2 texcoord_;

void main() {
  gl_Position = P * MV * vec4(position, 1.0);
  color_ = color;
  texcoord_ = texcoord;
}
)";}

inline std::string al_default_frag_shader() { return R"(
#version 330

uniform vec4 uniformColor;
uniform float uniformColorMix;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;

uniform float tex0_mix;
uniform float tex1_mix;
uniform float tex2_mix;

in vec4 color_;
in vec2 texcoord_;

out vec4 frag_color;

void main() {
  vec4 plain_color = mix(color_, uniformColor, uniformColorMix);
  float total_tex_mix = min(tex0_mix + tex1_mix + tex2_mix, 1.0);
  vec4 tex0_color = texture(tex0, texcoord_) * tex0_mix;
  vec4 tex1_color = texture(tex1, texcoord_) * tex1_mix;
  vec4 tex2_color = texture(tex2, texcoord_) * tex2_mix;
  vec4 tex_color = tex0_color + tex1_color + tex2_color;
  frag_color = mix(plain_color, tex_color, total_tex_mix);
}
)";}

#endif
