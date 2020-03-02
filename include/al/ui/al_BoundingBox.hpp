
#ifndef AL_BOUNDINGBOX_HPP
#define AL_BOUNDINGBOX_HPP

#include <limits>

#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Vec.hpp"

namespace al {

/// @defgroup UI UI

struct BoundingBoxData {
  Vec3f min, max;
  Vec3f cen, dim;

  BoundingBoxData();
  BoundingBoxData(const Vec3f &min_, const Vec3f &max_);

  void set(const Vec3f &min_, const Vec3f &max_);

  void setCenterDim(const Vec3f &cen_, const Vec3f &dim_);

  bool contains(const Vec3d &p);

  inline void reset();

  /**
   * @brief reset box to float value inverse extremes
   *
   * This allows building the bounding box incremementally using includePoint()
   */
  inline void resetInv();

  inline void includePoint(al::Vec3f &pos);
};

/// BoundingBox
/// @ingroup UI
struct BoundingBox : public BoundingBoxData {
  Mesh mesh, tics, gridMesh[2];
  float glUnitLength;
  float ticsPerUnit;

  BoundingBox();

  BoundingBox(const Vec3f &min_, const Vec3f &max_);

  // set bounds from mesh
  void set(const Mesh &mesh);

  void set(const Vec3f &min_, const Vec3f &max_);

  void setCenterDim(const Vec3f &cen_, const Vec3f &dim_);

  Mesh &getMesh();

  void draw(Graphics &g, bool drawTics = false);

  // tic marks
  Mesh &getTics();
};

}  // namespace al

#endif  // AL_BOUNDINGBOX_HPP
