#ifndef INLCUDE_AL_PERPROJECTION_HPP
#define INLCUDE_AL_PERPROJECTION_HPP

#include "al/core/math/al_Matrix4.hpp"
#include "al/core/graphics/al_Viewpoint.hpp"
#include "al/core/graphics/al_FBO.hpp"
#include "al/core/graphics/al_Shader.hpp"
#include "al/core/graphics/al_Texture.hpp"
#include "al/core/graphics/al_VAOMesh.hpp"
#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/graphics/al_Shapes.hpp"

#include <fstream>
#include <iostream>
#include <vector>

namespace al {

class PerProjectionRenderConstants {
public:
  static int const sampletex_binding_point = 10;
  static int const textures_bidning_point = 11;
};

inline std::string perprojection_vert() { return R"(
#version 330
uniform mat4 MV;
uniform mat4 P;  // projection matrix is a combination of rotation and projection here

// @eye_sep: the eye parallax distance.
//  This will be zero for mono, and positive/negative for right/left eyes.
uniform float eye_sep;

// @foc_len: the radius of the sphere in OpenGL units.
//  This will be infinity for the original layout (we default to 1e10).
uniform float foc_len;

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
// layout (location = 2) in vec2 texcoord;
// layout (location = 3) in vec3 normal;

out vec4 color_;
// out vec2 texcoord_;
// out vec3 normal_;

vec4 displace(in vec4 vertex) {
  float l = length(vertex.xz);
  vec3 vn = normalize(vertex.xyz);

  // Precise formula.
  float omni_eyeSep = eye_sep;
  float omni_radius = foc_len;
  float displacement = omni_eyeSep * (omni_radius * omni_radius - sqrt(l * l * omni_radius * omni_radius + omni_eyeSep * omni_eyeSep * (omni_radius * omni_radius - l * l))) / (omni_radius * omni_radius - omni_eyeSep * omni_eyeSep);

  // Approximation, safe if omni_radius / omni_eyeSep is very large, which is true for the allosphere.
  // float displacement = omni_eyeSep * (1.0 - l / omni_radius);

  // Displace vertex.
  return vertex + vec4(displacement * vn.z, 0.0, displacement * -vn.x, 0.0);
}

void main() {
  vec4 vertex = MV * vec4(position, 1.0);
  gl_Position = P * displace(vertex);
  color_ = color;
  // texcoord_ = texcoord;
  // normal_ = normal;
}
)";}

inline std::string perprojection_frag() { return R"(
#version 330
// uniform sampler2D tex0;
// uniform float tex0_mix;
// uniform float light_mix;

in vec4 color_;
// in vec2 texcoord_;
// in vec3 normal_;

out vec4 frag_color;

void main() {
  // vec4 tex_color0 = texture(tex0, texcoord_);
  // vec4 light_color = vec4(normal_, 1.0); // TODO
  // vec4 final_color = mix(mix(color_, tex_color0, tex0_mix), light_color, light_mix);
  frag_color = color_;
}
)";}

inline std::string perprojection_samplevert() { return R"(
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

inline std::string perprojection_samplefrag() { return R"(
#version 330
uniform sampler2D sample_tex;
uniform sampler2D color_tex;
uniform mat4 R;
uniform float tanFovDiv2;
in vec2 texcoord_;
out vec4 frag_color;
void main() {
  vec3 dir = texture(sample_tex, texcoord_).rgb;
  vec3 p_coord = (R * vec4(dir, 0)).xyz;
  p_coord.xy /= -p_coord.z;
  p_coord.xy /= tanFovDiv2;
  vec4 cube_color = texture(color_tex, p_coord.xy / 2.0 + 0.5);
  frag_color = cube_color;
}
)";}

class ProjectionViewport {
public:
  std::string id;
  int width;
  int height;
  float b, h, l, w;
  int active;
  std::string filepath;
  int warp_width, warp_height;
  std::vector<Vec3f> warp_data;
  std::vector<float> blend_data;
};

class WarpBlendData {
public:
  std::vector<ProjectionViewport> viewports;

  void load_allosphere_calibration(const char* path, const char* hostname) {
    std::ifstream config((std::string(path) + "/" + std::string(hostname) + ".txt").c_str());

    std::string id;
    if(config >> id) {
      bool got_next = false;
      do {
        ProjectionViewport vp;
        std::string tag;

        config >> id;
        got_next = false;
        while(config >> tag) {
          vp.id = id;
          if(tag == "id") {
            got_next = true;
            break;
          }
          if(tag == "width") config >> vp.width;
          else if(tag == "height") config >> vp.height;
          else if(tag == "b") config >> vp.b;
          else if(tag == "h") config >> vp.h;
          else if(tag == "l") config >> vp.l;
          else if(tag == "w") config >> vp.w;
          else if(tag == "active") config >> vp.active;
          else if(tag == "filepath") config >> vp.filepath;
          else std::cout << "unrecognized tag: " << tag << std::endl;
        }
        // std::cout << id << std::endl << vp.width << ", " << vp.height << std::endl;
        // std::cout << vp.b << ", " << vp.h << ", " << vp.l << ", " << vp.w << ", " << vp.active << ", " << vp.filepath << std::endl;
        viewports.push_back(vp);
      } while(got_next);
    }

    // Load warp data
    for(int i = 0; i < viewports.size(); i++) {
      ProjectionViewport& vp = viewports[i];
      std::ifstream file((std::string(path) + "/map3D" + vp.id + ".bin"), std::ios::in | std::ios::binary);

      if(!file) {
        std::cout << "could not open file: " << vp.filepath << std::endl;
        continue;
      }
      int32_t warp_size[2];
      file.read((char*)warp_size, 8);
      vp.warp_width = warp_size[1];
      vp.warp_height = warp_size[0] / 3;

      // std::cout << vp.warp_width << ", " << vp.warp_height << std::endl;

      vp.warp_data.resize(vp.warp_width * vp.warp_height);

      float* buf = new float[vp.warp_width * vp.warp_height];
      file.read((char*)buf, sizeof(float) * vp.warp_width * vp.warp_height); // x
      for(int j = 0; j < vp.warp_width * vp.warp_height; j++) {
          vp.warp_data[j].x = buf[j];
      }
      file.read((char*)buf, sizeof(float) * vp.warp_width * vp.warp_height); // y
      for(int j = 0; j < vp.warp_width * vp.warp_height; j++) {
          vp.warp_data[j].y = buf[j];
      }
      file.read((char*)buf, sizeof(float) * vp.warp_width * vp.warp_height); // z
      for(int j = 0; j < vp.warp_width * vp.warp_height; j++) {
          vp.warp_data[j].z = buf[j];
      }
      // Flip y axis.
      for(int y = 0; y < vp.warp_height / 2; y++) {
          Vec3f* row1 = &vp.warp_data[0] + y * vp.warp_width;
          Vec3f* row2 = &vp.warp_data[0] + (vp.warp_height - 1 - y) * vp.warp_width;
          for(int x = 0; x < vp.warp_width; x++) {
              Vec3f tmp = row1[x];
              row1[x] = row2[x];
              row2[x] = tmp;
          }
      }

      delete[] buf;
    }
  }
};


class PerProjectionRender {
public:
  struct ProjectionInfo {
    std::shared_ptr<Texture> texture, warp_texture;
    Mat4f pc_matrix, r_matrix;
    float tanFovDiv2;
  };

  WarpBlendData warpblend_;
  int res_;
  Pose pose_;
  Viewpoint view_ {pose_};
  Viewport viewport_;
  std::vector<ProjectionInfo> projection_infos_;
  RBO rbo_;
  FBO fbo_;
  // ShaderProgram pp_shader_;
  ShaderProgram composite_shader_;
  Lens lens_;
  Graphics* g;
  VAOMesh texquad;
  
  // instead of push/pop
  Lens prev_lens_;
  ShaderProgram* prev_shader_;

  Mat4f get_rotation_matrix(Vec3f axis, float angle) {
    Mat4f r;
    float cosT = cos(angle);
    float sinT = sin(angle);
    float X = axis.x, Y = axis.y, Z = axis.z;
    r.set(
      cosT + X * X * (1 - cosT), X * Y * (1 - cosT) - Z * sinT, X * Z * (1 - cosT) + Y * sinT, 0,
      Y * X * (1 - cosT) + Z * sinT, cosT + Y * Y * (1 - cosT), Y * Z * (1 - cosT) - X * sinT, 0,
      Z * X * (1 - cosT) - Y * sinT, Z * Y * (1 - cosT) + X * sinT, cosT + Z * Z * (1 - cosT), 0,
      0, 0, 0, 1
    );
    return r;
  }

  void init(int res=1024, float near=0.1, float far=100, float radius = 1e10) {
    res_ = res;
    lens_.focalLength(radius);
    // radius_ = radius;
    viewport_.set(0, 0, res_, res_);

    projection_infos_.resize(warpblend_.viewports.size());
    for(int index = 0; index < warpblend_.viewports.size(); index++) {
      const ProjectionViewport& vp = warpblend_.viewports[index];
      ProjectionInfo& info = projection_infos_[index];
      info.texture.reset(new Texture());
      info.texture->create2D(res_, res_, GL_RGBA32F, GL_RGBA, GL_FLOAT);

      // Determine projection dimensions.
      // First determine the central direction.
      Vec3f direction(0, 0, 0);
      for(int i = 0; i < vp.warp_width * vp.warp_height; i++) {
        Vec3f d = vp.warp_data[i];
        direction += d;
      }
      direction = direction.normalize();
      // Determine the radius.
      float dot_max = 1;
      for(int i = 0; i < vp.warp_width * vp.warp_height; i++) {
        Vec3f d = vp.warp_data[i];
        d = d.normalize();
        float dot = d.dot(direction);
        if(dot_max > dot) dot_max = dot;
      }
      if(dot_max < 0.1) {
        // The angle is too wide, throw an error.
        std::cout << "unable to use per-projection mode, viewport angle too large." << std::endl;
      }
      float fov = std::acos(dot_max) * 2.0f;
      // std::cout << fov << std::endl;
        // ProjectionTexture& pt = tex_projections_[vp];
      Vec3f rotation_axis = Vec3f(0, 0, -1).cross(direction);
      rotation_axis = rotation_axis.normalize();
      float rotation_angle = std::acos(Vec3f(0, 0, -1).dot(direction));
      Mat4f rmat = get_rotation_matrix(rotation_axis, -rotation_angle);
      Mat4f proj;
      proj.set(
        1.0f / std::tan(fov / 2.0f), 0, 0, 0,
        0, 1.0f / std::tan(fov / 2.0f), 0, 0,
        0, 0, (near + far) / (near - far), (2.0f * near * far) / (near - far),
        0, 0, -1, 0
      );
      Mat4f::multiply(info.pc_matrix, proj, rmat);
      info.r_matrix = rmat;
      info.tanFovDiv2 = tan(fov / 2.0);

      info.warp_texture.reset(new Texture());
      info.warp_texture->create2D(vp.warp_width, vp.warp_height, GL_RGBA32F, GL_RGB, GL_FLOAT);
      info.warp_texture->submit(vp.warp_data.data());
    }
    rbo_.create(res_, res_);
    fbo_.bind();
    fbo_.attachTexture2D(*projection_infos_[0].texture);
    fbo_.attachRBO(rbo_);
    fbo_.unbind();
    // pp_shader_.compile(perprojection_vert(), perprojection_frag());
    // pp_shader_.begin();
    // pp_shader_.uniform("omni_radius", radius_);
    // pp_shader_.end();

    composite_shader_.compile(perprojection_samplevert(), perprojection_samplefrag());
    composite_shader_.begin();
    composite_shader_.uniform("sample_tex", PerProjectionRenderConstants::sampletex_binding_point);
    composite_shader_.uniform("color_tex", PerProjectionRenderConstants::textures_bidning_point);
    composite_shader_.end();

    // prepare textured quad to fill viewport with the result
    addTexQuad(texquad);
    texquad.update();
  }

  void load_calibration_data(const char* path, const char* hostname) {
    warpblend_.load_allosphere_calibration(path, hostname);
  }

  void begin(Graphics& graphics) {
    g = &graphics;
    g->pushFramebuffer(fbo_);
    g->pushViewport(viewport_);
    g->pushViewMatrix(view_mat(pose_));
    g->pushProjMatrix();
    // TODO: pushLens and pushShader
    prev_lens_ = g->lens();
    g->lens(lens_);
    prev_shader_ = g->shaderPtr();
    // g->shader(pp_shader_);
  }

  // must only use between begin and end
  void set_eye(int i) {
    if (i == 0) g->eye(Graphics::LEFT_EYE);
    else if (i == 1) g->eye(Graphics::RIGHT_EYE);
    else if (i == -1) g->eye(Graphics::MONO_EYE);

    // g->shader().uniform("omni_eyeSep", 0.0);
  }

  int num_eyes() {
    return 1;
  }

  int num_projections() {
    return projection_infos_.size();
  }

  // must only use between begin and end
  void set_projection(int index) {
    g->projMatrix(projection_infos_[index].pc_matrix);
    fbo_.attachTexture2D(*projection_infos_[index].texture);
  }

  void end() {
    g->popFramebuffer();
    g->popViewport();
    g->popViewMatrix();
    g->popProjMatrix();
    g->lens(prev_lens_);
    if (prev_shader_ != nullptr) {
      g->shader(*prev_shader_);
    }
    // fbo_.end();
  }
  
  void pose(Pose const& p) {
    pose_ = p;
  }

  void composite(Graphics& g) {
    g.pushCamera(Viewpoint::IDENTITY);
    g.pushViewport();
    GLint dims[4];
    glGetIntegerv(GL_VIEWPORT, dims);
    int width = dims[2];
    int height = dims[3];
    g.shader(composite_shader_);
    for(int i = 0; i < warpblend_.viewports.size(); i++) {
      const ProjectionViewport& vp = warpblend_.viewports[i];

      g.viewport(vp.l * width, vp.b * height, vp.w * width, vp.h * height);
      projection_infos_[i].warp_texture->bind(PerProjectionRenderConstants::sampletex_binding_point);
      projection_infos_[i].texture->bind(PerProjectionRenderConstants::textures_bidning_point);
      g.shader().uniform("R", projection_infos_[i].r_matrix);
      g.shader().uniform("tanFovDiv2", projection_infos_[i].tanFovDiv2);
      g.draw(texquad); // fill viewport
      projection_infos_[i].warp_texture->unbind(PerProjectionRenderConstants::sampletex_binding_point);
      projection_infos_[i].texture->unbind(PerProjectionRenderConstants::textures_bidning_point);
    }
    g.popViewport();
    g.popCamera();
  }
};
}

#endif