#include "al/core/graphics/al_DefaultShaders.hpp"
#include "al/core/graphics/al_GLEW.hpp"
#include <string>
#include <iostream>

void al_print_lighting_uniforms(lighting_shader_uniforms const& u,
                                std::string name = "shader")
{
  std::cout << name << " locations:" << std::endl;
  std::cout << "\tglobal ambient: " << u.global_ambient << std::endl;
  std::cout << "\tnormal matrix: " << u.normal_matrix << std::endl;
  std::cout << "\tnumber of lights: " << u.num_lights << std::endl;     
  for (int i = 0; i < u.num_lights; i += 1) {
      auto const& l = u.lights[i];
      std::cout << "\t\tlight " << i << std::endl;
      std::cout << "\t\t\tambient " << l.ambient << std::endl;
      std::cout << "\t\t\tdiffuse " << l.diffuse << std::endl;
      std::cout << "\t\t\tspecular " << l.specular << std::endl;
      std::cout << "\t\t\tposition " << l.position << std::endl;
      // std::cout << "\t\t\tattenuation " << l.atten << std::endl;
  }
  std::cout << "\thas material: " << (u.has_material? "yes" : "no") << std::endl;
  if (u.has_material) {
      auto const& m = u.material;
      std::cout << "\t\tambient: " << m.ambient << std::endl;
      std::cout << "\t\tdiffuse: " << m.diffuse << std::endl;
      std::cout << "\t\tspecular: " << m.specular << std::endl;
      std::cout << "\t\tshininess: " << m.shininess << std::endl;
      // std::cout << "\t\temission: " << m.emission << std::endl;
  }
}

lighting_shader_uniforms al_get_lighting_uniform_locations(al::ShaderProgram& s,
                                                           bool print_result=false)
{
    using namespace std::string_literals;
    lighting_shader_uniforms u;
    /*
        glGetUniformLocation
            This function returns -1 if name does not correspond to
            an active uniform variable in program
            or if name starts with the reserved prefix "gl_".
    */
    u.global_ambient = glGetUniformLocation(s.id(), "light_global_ambient");
    u.normal_matrix = glGetUniformLocation(s.id(), "N");
    for (int i = 0; i < al_max_num_lights(); i += 1) {
        per_light_uniform_locations l;
        auto light_i = "light"s + std::to_string(i); // light0, light1, ...
        l.ambient = glGetUniformLocation(s.id(), (light_i + "_ambient"s).c_str());
        l.diffuse = glGetUniformLocation(s.id(), (light_i + "_diffuse"s).c_str());
        l.specular = glGetUniformLocation(s.id(), (light_i + "_specular"s).c_str());
        l.position = glGetUniformLocation(s.id(), (light_i + "_eye"s).c_str());
        // l.atten = s.getUniformLocation("");
        if (l.ambient != -1 && l.diffuse != -1 && l.specular != -1 && l.position != -1) {
            u.lights.push_back(l);
            u.num_lights = i + 1;
        }
        else break;
    }
    auto& m = u.material;
    m.ambient = glGetUniformLocation(s.id(), "material_ambient");
    m.diffuse = glGetUniformLocation(s.id(), "material_diffuse");
    m.specular = glGetUniformLocation(s.id(), "material_specular");
    m.shininess = glGetUniformLocation(s.id(), "material_shininess");
    // m.emission = s.getUniformLocation("");
    if (m.ambient != -1 && m.diffuse != -1 && m.specular != -1 && m.shininess != -1) {
        u.has_material = true;
    }

    if (print_result) {
      al_print_lighting_uniforms(u);
    }

    return u;
}

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
		case ShaderType::LIGHTING_COLOR:
			s.compile(al_lighting_color_vert_shader(), al_lighting_color_frag_shader());
			return;
		case ShaderType::LIGHTING_MESH:
			s.compile(al_lighting_mesh_vert_shader(), al_lighting_mesh_frag_shader());
			return;
		case ShaderType::LIGHTING_TEXTURE:
			s.compile(al_lighting_tex_vert_shader(), al_lighting_tex_frag_shader());
			return;
		case ShaderType::LIGHTING_MATERIAL:
			s.compile(al_lighting_material_vert_shader(), al_lighting_material_frag_shader());
			return;
	}
}

}