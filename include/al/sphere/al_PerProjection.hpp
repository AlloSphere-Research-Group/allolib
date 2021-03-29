#ifndef INLCUDE_AL_PERPROJECTION_HPP
#define INLCUDE_AL_PERPROJECTION_HPP

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

#include "al/graphics/al_FBO.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Shader.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_Texture.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/graphics/al_Viewpoint.hpp"
#include "al/math/al_Matrix4.hpp"

#ifdef AL_WINDOWS
#undef near
#undef far
#endif

namespace al {

Mat4f get_cube_mat(int face);

struct bhlw {
  float b;
  float h;
  float l;
  float w;
};

bhlw viewport_for_cubemap_face(int idx);

class PerProjectionRenderConstants {
public:
  static int const sampletex_binding_point = 10;
  static int const textures_bidning_point = 11;
};

inline std::string perprojection_samplevert() {
  return R"(
#version 330
uniform mat4 al_ModelViewMatrix;
uniform mat4 al_ProjectionMatrix;

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texcoord;

out vec2 texcoord_;

void main() {
gl_Position = al_ProjectionMatrix * al_ModelViewMatrix * vec4(position, 1.0);
texcoord_ = texcoord;
}
)";
}

inline std::string perprojection_samplefrag() {
  return R"(
#version 330
uniform sampler2D sample_tex;
uniform sampler2D color_tex;
uniform mat4 R;
uniform float tanFovDiv2;
in vec2 texcoord_;
out vec4 frag_color;
void main() {
vec4 sample = texture(sample_tex, texcoord_);
vec3 dir = sample.rgb;
vec3 p_coord = (R * vec4(dir, 0)).xyz;
p_coord.xy /= -p_coord.z;
p_coord.xy /= tanFovDiv2;
vec3 sampled_color = texture(color_tex, p_coord.xy / 2.0 + 0.5).rgb;
frag_color = vec4(sampled_color * sample.a, 1.0);
}
)";
}

class ProjectionViewport {
public:
  std::string id;
  float b, h, l, w;
  int active;
  std::string filepath;
  int width, height;
  // std::vector<Vec3f> warp_data;
  // std::vector<float> blend_data;
  std::vector<Vec4f> warp_and_blend_data;
};

class WarpBlendData {
public:
  std::vector<ProjectionViewport> viewports;
  void load_allosphere_calibration(const char *path, const char *hostname);
  void load_desktop_mode_calibration();
};

class PerProjectionRender {
public:
  struct ProjectionInfo {
    std::shared_ptr<Texture> texture[2];
    std::shared_ptr<Texture> warp_texture;
    Mat4f pc_matrix, p_matrix, r_matrix;
    float tanFovDiv2; // tan(fov)/2
  };

  WarpBlendData warpblend_;
  int res_ = 4096;
  Pose pose_;
  Viewpoint view_{pose_};
  Viewport viewport_;
  std::vector<ProjectionInfo> projection_infos_;
  RBO rbo_;
  FBO fbo_;
  // ShaderProgram pp_shader_;
  ShaderProgram composite_shader_;
  Lens lens_;
  Graphics *g;
  VAOMesh texquad;
  bool calibration_loaded = false;
  bool did_begin = false;
  int current_eye = 0;
  int current_projection = 0;

  // for saving previous settings
  Lens prev_lens_;
  ShaderProgram *prev_shader_;

  Mat4f get_rotation_matrix(Vec3f axis, float angle);

  // load_calibration_data needed to be called before this function
  // also projection_infos_.resize(warpblend_.viewports.size());
  void update_resolution(int resolution);

  // void sphereRadius(float r) {
  // lens_.focalLength(r);
  // if (did_begin) g->lens(lens_);
  //}

  // void init(float near=0.1, float far=1000, float radius = 1e10)
  void init(const al::Lens &lens);

  void load_calibration_data(const char *path, const char *hostname);

  // void load_and_init_as_desktop_config(float near=0.1, float far=1000, float
  // radius = 1e10) {
  void load_and_init_as_desktop_config(const al::Lens &lens);

  // use when near and far changed (only use after init)
  // fov value of lens will not be used in omni drawing
  // focallength value will be updated when drawing is actually done
  // by uploading to shader as glsl-uniform value
  void updateLens(const Lens &lens);

  // changing lens and pose after begin does not have effect on omni rendering
  void begin(Graphics &graphics, const al::Lens &lens, const al::Pose pose);

  // must only be used between begin and end
  void set_eye(int i);

  int num_projections() { return projection_infos_.size(); }

  // must only use between begin and end
  void set_projection(int index);

  Mat4f get_r_mat_for_current_projection() {
    return projection_infos_[current_projection].r_matrix;
  }

  Mat4f get_p_mat_for_current_projection() {
    return projection_infos_[current_projection].p_matrix;
  }

  void end();

  // void pose(Pose const& p) { pose_ = p; if (did_begin)
  // g->viewMatrix(view_mat(pose_)); } Pose& pose() { return pose_; } Pose
  // const& pose() const { return pose_; }

  void composite(Graphics &g, int eye = 0);

  void composite_desktop(Graphics &g, int eye = 0);
};
} // namespace al

#endif
