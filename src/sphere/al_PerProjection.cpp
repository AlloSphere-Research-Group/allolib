#include "al/sphere/al_PerProjection.hpp"

al::Mat4f al::get_cube_mat(int face) {
  switch (face) {
    // GL_TEXTURE_CUBE_MAP_POSITIVE_X
    // vertex.xyz = vec3(-vertex.z, -vertex.y, -vertex.x);
    case 0:
      return Mat4f{0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 0, 0, 0, 1};
      // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
      // vertex.xyz = vec3(vertex.z, -vertex.y, vertex.x);
    case 1:
      return Mat4f{0, 0, 1, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1};
      // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
      // vertex.xyz = vec3(vertex.x, vertex.z, -vertex.y);
    case 2:
      return Mat4f{1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1};
      // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
      // vertex.xyz = vec3(vertex.x, -vertex.z, vertex.y);
    case 3:
      return Mat4f{1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1};
      // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
      // vertex.xyz = vec3(vertex.x, -vertex.y, -vertex.z);
    case 4:
      return Mat4f{1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1};
      // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
      // vertex.xyz = vec3(-vertex.x, -vertex.y, vertex.z);
    case 5:
      return Mat4f{-1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
  }
  return Mat4f::identity();
}

al::bhlw al::viewport_for_cubemap_face(int idx) {
  /*
    _________
    | |2|   |
    |1|5|0|4|
    | |3|   |
    '''''''''
  */
  bhlw v;
  v.h = 0.33f;
  v.w = 0.25f;
  if (idx == 0) {
    v.b = 0.33f;
    v.l = 0.5f;
  } else if (idx == 1) {
    v.b = 0.33f;
    v.l = 0.0f;
  } else if (idx == 2) {
    v.b = 0.66f;
    v.l = 0.25f;
  } else if (idx == 3) {
    v.b = 0.0f;
    v.l = 0.25f;
  } else if (idx == 4) {
    v.b = 0.33f;
    v.l = 0.75f;
  } else if (idx == 5) {
    v.b = 0.33f;
    v.l = 0.25f;
  }
  return v;
}
