#ifndef AL_SPHERE_SHELL_HPP
#define AL_SPHERE_SHELL_HPP

/**
 * @file al_SphereShell.hpp
 * @brief Sphere-shell pick/drag helpers (relative grab, ray intersection).
 */

#include <algorithm>
#include <cmath>

#include "al/math/al_Quat.hpp"
#include "al/math/al_Ray.hpp"
#include "al/spatial/al_Pose.hpp"

namespace al {
namespace sphere {

inline bool intersectRaySphere(Rayd ray, double radius, const Vec3d &nearHint,
                               Vec3d &hit) {
  if (radius <= 1e-4) {
    return false;
  }
  const Vec3d o = ray.origin();
  const Vec3d d = ray.direction();
  const double a = d.dot(d);
  const double b = 2.0 * o.dot(d);
  const double c = o.dot(o) - radius * radius;
  const double disc = b * b - 4.0 * a * c;
  if (disc < 0.0) {
    return false;
  }
  const double s = std::sqrt(disc);
  const double inv2a = 0.5 / a;
  const double t0 = (-b - s) * inv2a;
  const double t1 = (-b + s) * inv2a;
  const Vec3d p0 = o + d * t0;
  const Vec3d p1 = o + d * t1;
  if ((p0 - nearHint).magSqr() <= (p1 - nearHint).magSqr()) {
    hit = p0;
  } else {
    hit = p1;
  }
  return true;
}

/// Billboard pose facing origin from a point on the shell.
inline Pose poseOnShell(Vec3f pos, float radiusM) {
  const float r = pos.mag();
  if (r < 1e-4f) {
    pos = Vec3f(radiusM, 0, 0);
  } else {
    pos *= (radiusM / r);
  }
  Pose pose;
  pose.pos() = pos;
  Vec3f forward = pos;
  forward.normalize();
  pose.quat() = Quatf::getBillboardRotation(-forward, Vec3f(0.f, 1.f, 0.f));
  return pose;
}

/**
 * Relative grab on a sphere shell: entity stays offset from the pick ray hit
 * instead of snapping its center to the cursor.
 */
struct ShellGrab {
  float radius{1.f};
  Quatf hitToEntity{1, 0, 0, 0}; ///< rotates hitDir → entityDir at grab start
  bool active{false};

  void begin(Vec3f hit, Vec3f entityPos, float radiusM) {
    radius = radiusM;
    Vec3f hitDir = hit;
    Vec3f entDir = entityPos;
    if (hitDir.mag() < 1e-4f || entDir.mag() < 1e-4f) {
      hitToEntity = Quatf(1, 0, 0, 0);
      active = true;
      return;
    }
    hitDir.normalize();
    entDir.normalize();
    hitToEntity = Quatf::getRotationTo(hitDir, entDir);
    active = true;
  }

  void end() { active = false; }

  /// Map current ray–sphere hit to entity pose (relative to grab).
  Pose updateFromHit(Vec3f hit) const {
    Vec3f hitDir = hit;
    if (hitDir.mag() < 1e-4f) {
      hitDir = Vec3f(1, 0, 0);
    } else {
      hitDir.normalize();
    }
    Vec3f entDir = hitToEntity.rotate(hitDir);
    if (entDir.mag() < 1e-4f) {
      entDir = hitDir;
    } else {
      entDir.normalize();
    }
    return poseOnShell(entDir * radius, radius);
  }
};

/// Screen-space drag gate: ignore tiny motions so clicks don't translate.
struct DragThreshold {
  float pixels{6.f};
  int startX{0};
  int startY{0};
  bool armed{false};
  bool dragging{false};

  void press(int x, int y) {
    startX = x;
    startY = y;
    armed = true;
    dragging = false;
  }

  /// Returns true once movement exceeds threshold (sticky).
  bool considerMove(int x, int y) {
    if (!armed) {
      return dragging;
    }
    if (dragging) {
      return true;
    }
    const float dx = static_cast<float>(x - startX);
    const float dy = static_cast<float>(y - startY);
    if (dx * dx + dy * dy >= pixels * pixels) {
      dragging = true;
    }
    return dragging;
  }

  void release() {
    armed = false;
    dragging = false;
  }

  bool wasDrag() const { return dragging; }
};

} // namespace sphere
} // namespace al

#endif
