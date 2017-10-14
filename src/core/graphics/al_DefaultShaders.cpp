#include "al/core/graphics/al_DefaultShaders.hpp"

namespace al {

void compileDefaultShader(ShaderProgram& s, ShaderType type)
{
	switch (type) {
		case ShaderType::COLOR:
			s.compile(al_color_vert_shader(), al_color_frag_shader());
			return;
		case ShaderType::MESH:
			s.compile(al_mesh_vert_shader(), al_mesh_frag_shader());
			return;
		case ShaderType::TEXTURE:
			s.compile(al_tex_vert_shader(), al_tex_frag_shader());
			return;
		case ShaderType::LIGHTING:
		default: break;
	}
}

}