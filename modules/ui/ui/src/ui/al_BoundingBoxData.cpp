#include "al/ui/al_BoundingBoxData.hpp"

using namespace al;

BoundingBoxData::BoundingBoxData() { reset(); }

BoundingBoxData::BoundingBoxData(const Vec3f &min_, const Vec3f &max_)
    : min(min_), max(max_) {
  dim = max - min;
  Vec3f halfDim = dim / 2;
  cen = min + halfDim;
}

void BoundingBoxData::set(const Vec3f &min_, const Vec3f &max_) {
  min.set(min_);
  max.set(max_);
  dim = max - min;
  Vec3f halfDim = dim / 2;
  cen = min + halfDim;
}

void BoundingBoxData::setCenterDim(const Vec3f &cen_, const Vec3f &dim_) {
  cen.set(cen_);
  dim.set(dim_);
  min = cen - dim / 2;
  max = cen + dim / 2;
}

bool BoundingBoxData::contains(const Vec3d &p) {
  if (p.x < min.x || p.x > max.x)
    return false;
  if (p.y < min.y || p.y > max.y)
    return false;
  if (p.z < min.z || p.z > max.z)
    return false;
  return true;
}

void BoundingBoxData::reset() {
  min = {0, 0, 0};
  max = {0, 0, 0};
}
