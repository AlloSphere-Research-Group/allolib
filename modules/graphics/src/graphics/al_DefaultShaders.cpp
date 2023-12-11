#include "al/graphics/al_DefaultShaders.hpp"

#include <iostream>

#include "al/graphics/al_OpenGL.hpp"

void al_print_lighting_uniforms(lighting_shader_uniforms const& u,
                                std::string name) {
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
    std::cout << "\t\t\tenabled " << l.enabled << std::endl;
    // std::cout << "\t\t\tattenuation " << l.atten << std::endl;
  }
  std::cout << "\thas material: " << (u.has_material ? "yes" : "no")
            << std::endl;
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
                                                           bool print_result) {
  using namespace std::string_literals;
  lighting_shader_uniforms u;
  /*
      glGetUniformLocation
          This function returns -1 if name does not correspond to
          an active uniform variable in program
          or if name starts with the reserved prefix "gl_".
  */
  u.global_ambient = glGetUniformLocation(s.id(), "light_global_ambient");
  u.normal_matrix = glGetUniformLocation(s.id(), "al_NormalMatrix");
  for (int i = 0; i < al_max_num_lights(); i += 1) {
    per_light_uniform_locations l;
    auto light_i = "light"s + std::to_string(i);  // light0, light1, ...
    l.ambient = glGetUniformLocation(s.id(), (light_i + "_ambient"s).c_str());
    l.diffuse = glGetUniformLocation(s.id(), (light_i + "_diffuse"s).c_str());
    l.specular = glGetUniformLocation(s.id(), (light_i + "_specular"s).c_str());
    l.position = glGetUniformLocation(s.id(), (light_i + "_eye"s).c_str());
    l.enabled = glGetUniformLocation(s.id(), (light_i + "_enabled"s).c_str());
    // l.atten = s.getUniformLocation("");
    if (l.ambient != -1 && l.diffuse != -1 && l.specular != -1 &&
        l.position != -1) {
      u.lights.push_back(l);
      u.num_lights = i + 1;
    } else
      break;
  }
  auto& m = u.material;
  m.ambient = glGetUniformLocation(s.id(), "material_ambient");
  m.diffuse = glGetUniformLocation(s.id(), "material_diffuse");
  m.specular = glGetUniformLocation(s.id(), "material_specular");
  m.shininess = glGetUniformLocation(s.id(), "material_shininess");
  // m.emission = s.getUniformLocation("");
  if (m.ambient != -1 && m.diffuse != -1 && m.specular != -1 &&
      m.shininess != -1) {
    u.has_material = true;
  }

  if (print_result) {
    al_print_lighting_uniforms(u);
  }

  return u;
}

namespace al {

void compileDefaultShader(ShaderProgram& s, ShaderType type, bool is_omni) {
  switch (type) {
    case ShaderType::COLOR:
      s.compile(al_color_vert_shader(is_omni), al_color_frag_shader());
      return;
    case ShaderType::MESH:
      s.compile(al_mesh_vert_shader(is_omni), al_mesh_frag_shader());
      return;
    case ShaderType::TEXTURE:
      s.compile(al_tex_vert_shader(is_omni), al_tex_frag_shader());
      return;
    case ShaderType::LIGHTING_COLOR:
      s.compile(multilight_vert_shader(ShaderType::LIGHTING_COLOR, 1, is_omni),
                multilight_frag_shader(ShaderType::LIGHTING_COLOR, 1));
      return;
    case ShaderType::LIGHTING_MESH:
      s.compile(multilight_vert_shader(ShaderType::LIGHTING_MESH, 1, is_omni),
                multilight_frag_shader(ShaderType::LIGHTING_MESH, 1));
      return;
    case ShaderType::LIGHTING_TEXTURE:
      s.compile(
          multilight_vert_shader(ShaderType::LIGHTING_TEXTURE, 1, is_omni),
          multilight_frag_shader(ShaderType::LIGHTING_TEXTURE, 1));
      return;
    case ShaderType::LIGHTING_MATERIAL:
      s.compile(
          multilight_vert_shader(ShaderType::LIGHTING_MATERIAL, 1, is_omni),
          multilight_frag_shader(ShaderType::LIGHTING_MATERIAL, 1));
      return;
  }
}

// common lines in the beginning
std::string multilight_vert_header_common() {
  return R"(
uniform mat4 al_ModelViewMatrix;
uniform mat4 al_ProjectionMatrix;
uniform mat4 al_NormalMatrix; // normal matrix: transpose of inverse of al_ModelViewMatrix
uniform float eye_sep;
uniform float foc_len;
layout (location = 0) in vec3 position;
layout (location = 3) in vec3 normal;
out vec3 normal_eye;
out vec3 eye_dir;
)";
}

std::string multilight_vert_header_pertype(ShaderType type) {
  switch (type) {
    case ShaderType::COLOR:
    case ShaderType::LIGHTING_COLOR:
      return "";
    case ShaderType::MESH:
    case ShaderType::LIGHTING_MESH:
      return R"(
layout (location = 1) in vec4 color;
out vec4 color_;
)";
    case ShaderType::TEXTURE:
    case ShaderType::LIGHTING_TEXTURE:
      return R"(
layout (location = 2) in vec2 texcoord;
out vec2 texcoord_;
)";
    case ShaderType::LIGHTING_MATERIAL:
      return "";
  }
  return "";
}

std::string multilight_vert_header_perlight(int num_lights) {
  using namespace std::string_literals;
  std::string s = "\n";
  for (int i = 0; i < num_lights; i += 1) {
    auto light_i = "light"s + std::to_string(i);  // light0, light1, ...
    s += "uniform vec4 "s + light_i + "_eye;\n"s;
    s += "out vec3 "s + light_i + "_dir;\n"s;
    // s += "out float "s + light_i + "_dist;\n"s;
  }
  return s;
}

std::string multilight_vert_body_begin() {
  return R"(
void main()
{
    vec4 vert_eye = al_ModelViewMatrix * vec4(position, 1.0);
    if (eye_sep == 0) {
        gl_Position = al_ProjectionMatrix * al_ModelViewMatrix * vec4(position, 1.0);
    }
    else {
        gl_Position = al_ProjectionMatrix * stereo_displace(al_ModelViewMatrix * vec4(position, 1.0), eye_sep, foc_len);
    }
    normal_eye = (al_NormalMatrix * vec4(normalize(normal), 0.0)).xyz;
    eye_dir = -vert_eye.xyz;
)";  // does not close main function
}

std::string multilight_vert_body_pertype(ShaderType type) {
  switch (type) {
    case ShaderType::COLOR:
    case ShaderType::LIGHTING_COLOR:
      return "";
    case ShaderType::MESH:
    case ShaderType::LIGHTING_MESH:
      return R"(
    color_ = color;
)";
    case ShaderType::TEXTURE:
    case ShaderType::LIGHTING_TEXTURE:
      return R"(
    texcoord_ = texcoord;
)";
    case ShaderType::LIGHTING_MATERIAL:
      return "";
  }
  return "";
}

std::string multilight_vert_body_perlight(int num_lights) {
  using namespace std::string_literals;
  std::string s = "\n";
  for (int i = 0; i < num_lights; i += 1) {
    auto li = "light"s + std::to_string(i);  // light0, light1, ...
    // light0_dir = light0_eye.xyz - vert_eye.xyz * light0_eye.a;
    s += "    "s + li + "_dir = "s + li + "_eye.xyz - vert_eye.xyz * "s + li +
         "_eye.a;\n"s;
    // light0_dist = length(light0_dir);
    // s += "    "s + li + "_dist = length("s + li + "_dir);\n"s;
  }
  return s;
}

std::string multilight_vert_body_end() { return "}"; }

std::string multilight_frag_header_common() {
  return R"(#version 330
in vec3 normal_eye;
in vec3 eye_dir;
out vec4 frag_color;
uniform vec4 light_global_ambient;
uniform vec4 tint;
)";
}

std::string multilight_frag_header_pertype(ShaderType type) {
  switch (type) {
    case ShaderType::COLOR:
    case ShaderType::LIGHTING_COLOR:
      return R"(
uniform vec4 col0;
)";
    case ShaderType::MESH:
    case ShaderType::LIGHTING_MESH:
      return R"(
in vec4 color_;
)";
    case ShaderType::TEXTURE:
    case ShaderType::LIGHTING_TEXTURE:
      return R"(
in vec2 texcoord_;
uniform sampler2D tex0;
)";
    case ShaderType::LIGHTING_MATERIAL:
      return R"(
uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;
uniform float material_shininess;
)";
  }
  return "";
}

std::string multilight_frag_header_perlight(int num_lights) {
  using namespace std::string_literals;
  std::string s = "\n";
  for (int i = 0; i < num_lights; i += 1) {
    auto light_i = "light"s + std::to_string(i);  // light0, light1, ...
    s += "in vec3 "s + light_i + "_dir;\n"s;
    // s += "in float "s + light_i + "_dist;\n"s;
    s += "uniform vec4 "s + light_i + "_ambient;\n"s;
    s += "uniform vec4 "s + light_i + "_diffuse;\n"s;
    s += "uniform vec4 "s + light_i + "_specular;\n"s;
    s += "uniform float "s + light_i + "_enabled;\n"s;
    // TODO: atten
  }
  return s;
}

std::string multilight_frag_body_begin() {
  return R"(
void main()
{
    vec3 n = normalize(normal_eye); // normal
    vec3 e = normalize(eye_dir); // to eye
    vec3 lighting = vec3(0.0);
)";  // does not close main function
}

std::string multilight_frag_body_pertype(ShaderType type) {
  switch (type) {
    case ShaderType::COLOR:
    case ShaderType::LIGHTING_COLOR:
      return R"(
    vec3 ambient = col0.rgb * col0.a;
    vec3 diffuse = col0.rgb * col0.a;
    vec3 specular = col0.rgb * col0.a;
    float shininess = 5.0; // shininess 5.0 is OpenGL 2.x default value
)";
    case ShaderType::MESH:
    case ShaderType::LIGHTING_MESH:
      return R"(
    vec3 ambient = color_.rgb * color_.a;
    vec3 diffuse = color_.rgb * color_.a;
    vec3 specular = color_.rgb * color_.a;
    float shininess = 5.0; // shininess 5.0 is OpenGL 2.x default value
)";
    case ShaderType::TEXTURE:
    case ShaderType::LIGHTING_TEXTURE:
      return R"(
    vec4 tex = texture(tex0, texcoord_);
    vec3 ambient = tex.rgb * tex.a;
    vec3 diffuse = tex.rgb * tex.a;
    vec3 specular = tex.rgb * tex.a;
    float shininess = 5.0; // shininess 5.0 is OpenGL 2.x default value
)";
    case ShaderType::LIGHTING_MATERIAL:
      return R"(
    vec3 ambient = material_ambient.rgb * material_ambient.a;
    vec3 diffuse = material_diffuse.rgb * material_diffuse.a;
    vec3 specular = material_specular.rgb * material_specular.a;
    float shininess = material_shininess;
)";
  }
  return "";
}

std::string multilight_frag_body_perlight(int num_lights) {
  using namespace std::string_literals;
  std::string s = R"(
    vec3 d; // to light
    vec3 r; // reflection
    float n_d; // n dot d
    float e_r; // e dot r
)";
  for (int i = 0; i < num_lights; i += 1) {
    auto light_i = "light"s + std::to_string(i);  // light0, light1, ...
    s += "\n"s;
    s += "    d = normalize("s + light_i + "_dir);\n"s;
    s += "    r = -reflect(d, n);\n"s;
    s += "    n_d = max(dot(n, d), 0.0);\n"s;
    s += "    e_r = max(dot(e, r), 0.0);\n"s;
    s += "    lighting += "s + light_i + "_enabled * ("s + "ambient * "s +
         light_i + "_ambient.rgb + "s + "diffuse * "s + light_i +
         "_diffuse.rgb * n_d + "s + "specular * "s + light_i +
         "_specular.rgb * pow(e_r, shininess));\n"s;
    // TODO: atten
  }
  return s;
}

std::string multilight_frag_body_end() {
  return R"(
    lighting += ambient * light_global_ambient.rgb;
    frag_color = tint * vec4(lighting, 1.0);
}
)";
}

std::string multilight_vert_shader(ShaderType type, int num_lights,
                                   bool is_omni) {
  return al_default_shader_version_string() +
         al_default_vert_shader_stereo_functions(is_omni) +
         multilight_vert_header_common() +
         multilight_vert_header_pertype(type) +
         multilight_vert_header_perlight(num_lights) +
         multilight_vert_body_begin() + multilight_vert_body_pertype(type) +
         multilight_vert_body_perlight(num_lights) + multilight_vert_body_end();
}

std::string multilight_frag_shader(ShaderType type, int num_lights) {
  return multilight_frag_header_common() +
         multilight_frag_header_pertype(type) +
         multilight_frag_header_perlight(num_lights) +
         multilight_frag_body_begin() + multilight_frag_body_pertype(type) +
         multilight_frag_body_perlight(num_lights) + multilight_frag_body_end();
}

void compileMultiLightShader(ShaderProgram& s, ShaderType type, int num_lights,
                             bool is_omni) {
  s.compile(multilight_vert_shader(type, num_lights, is_omni),
            multilight_frag_shader(type, num_lights));
}

#if 0
std::string test_vert(ShaderType type, int num_lights) {
    return multilight_vert_shader(type, num_lights);
}

std::string test_frag(ShaderType type, int num_lights) {
    return multilight_frag_shader(type, num_lights);
}
#endif

}  // namespace al