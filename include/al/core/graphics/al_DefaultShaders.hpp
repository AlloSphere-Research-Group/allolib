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

namespace al {

enum class ShaderType : unsigned char {
	COLOR,
	MESH,
	TEXTURE,
	LIGHTING
};

void compileDefaultShader(ShaderProgram& s, ShaderType type);

}

#endif
