#ifndef __PICKABLE_HPP__
#define __PICKABLE_HPP__

#include <vector>

#include "al/graphics/al_Graphics.hpp"
#include "al/math/al_Ray.hpp"
#include "al/ui/al_BoundingBox.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterBundle.hpp"

namespace al {

enum PickEventType {
  Point,
  Pick,
  PickPose,
  Unpick,
  Drag,
  TranslateRay,
  RotateRay,
  RotateRayTrackball,
  RotateTurntable,
  RotatePose,
  Scale
};

struct PickEvent {
  PickEventType type;
  Rayd ray;
  Pose pose;
  Vec3f vec;
  float amount;

  PickEvent(PickEventType t, Rayd r) : type(t), ray(r) {}
  PickEvent(PickEventType t, Rayd r, Pose p) : type(t), ray(r), pose(p) {}
  PickEvent(PickEventType t, Rayd r, Vec3f v) : type(t), ray(r), vec(v) {}
  PickEvent(PickEventType t, Rayd r, Pose p, Vec3f v)
      : type(t), ray(r), pose(p), vec(v) {}
  PickEvent(PickEventType t, Rayd r, float v) : type(t), ray(r), amount(v) {}
  PickEvent(PickEventType t, Pose p) : type(t), pose(p) {}
  PickEvent(PickEventType t, float v) : type(t), amount(v) {}
};

struct Pickable;

struct Hit {
  bool hit;
  Rayd ray;
  double t;
  Pickable *p;

  Hit() : hit(false) {}
  Hit(bool h, Rayd r, double tt, Pickable *pp) : hit(h), ray(r), t(tt), p(pp) {}
  Vec3d operator()() { return hit ? ray(t) : Vec3d(); }
};

/// Pickable
/// @ingroup UI
struct Pickable {
  std::string name;

  ParameterBool hover{"hover", name}, selected{"selected", name};
  ParameterPose pose{"pose", name};
  ParameterVec3 scaleVec{"scaleVec", name};
  Parameter scale{"scale", name, 1.0f, 0.0f, 10.0f};
  ParameterBundle bundle{"pickable"};

  Pickable *parent = nullptr;
  std::vector<Pickable *> children;
  bool testChildren = true;
  bool containChildren = false;
  bool containedChild = false;
  int depth = 0;

  // initial values, and previous values
  Pose pose0, prevPose;
  Vec3f scale0, prevScale;

  Pickable();

  virtual ~Pickable() {}

  /// intersection test must be implemented
  virtual Hit intersect(Rayd r) = 0;
  virtual Hit intersect(Vec3d v) { return intersect(Rayd(v, Vec3d())); }

  /// override callback
  virtual bool onEvent(PickEvent e, Hit hit) { return false; }

  /// do interaction on self and children, call onEvent callbacks
  virtual bool event(PickEvent e);

  void clearSelection();

  bool intersects(Rayd &r) { return intersect(r).hit; }
  bool intersectsChild(Rayd &r);

  Hit intersectChildren(Rayd &r);

  void addChild(Pickable &pickable);
  void addChild(Pickable *p) { addChild(*p); }

  /// apply pickable pose transforms
  inline void pushMatrix(Graphics &g) {
    g.pushMatrix();
    g.translate(pose.get().pos());
    g.rotate(pose.get().quat());
    g.scale(scaleVec.get());
  }
  /// pop matrix.
  inline void popMatrix(Graphics &g) { g.popMatrix(); }

  void draw(
      Graphics &g, std::function<void(Pickable &p)> f = [](Pickable &p) {});

  void foreach (
      std::function<void(Pickable &p)> pre,
      std::function<void(Pickable &p)> post = [](Pickable & /*p*/) {}) {
    pre(*this);
    for (auto *c : children)
      c->foreach (pre, post);
    post(*this);
  }

  /// transform a ray in world space to local space
  Rayd transformRayLocal(const Rayd &ray);
  /// transfrom a vector in local space to world space
  Vec3f transformVecWorld(const Vec3f &v, float w = 1);
  /// transfrom a vector in world space to local space
  Vec3f transformVecLocal(const Vec3f &v, float w = 1);
};

/// Bounding Box PickableMesh
/// @ingroup UI
struct PickableBB : Pickable {
  Mesh *mesh{nullptr}; // pointer to mesh that is wrapped
  BoundingBox bb;      // original bounding box
  BoundingBox aabb; // axis aligned bounding box (after pose/scale transforms)

  // used for moving pickable naturally
  Vec3f selectPos;
  Vec3f selectOffset;
  Quatf selectQuat;
  float selectDist;

  PickableBB(std::string name_ = "") {
    Pickable::name = name_;
    bundle.name(name_);
  }
  PickableBB(Mesh &m) { set(m); }

  /// initialize bounding box;
  void set(Mesh &m);

  /// override base methods
  Hit intersect(Rayd r);
  // bool contains(Vec3d v){ auto p = transformVecLocal(v); return
  // bb.contains(p); }

  bool onEvent(PickEvent e, Hit h);

  void drawMesh(Graphics &g);

  void drawBB(Graphics &g);

  /// set the pickable's center position
  void setCenter(Vec3f &pos);

  /// set pickable's orientation maintaining same center position
  void setQuat(Quatf &q);

  /// intersect ray with pickable BoundingBox
  double intersectBB(Rayd localRay);

  /// intersect ray with pickable AxisAlignedBoundingBox
  double intersectAABB(Rayd ray);

  /// intersect ray with bounding sphere
  float intersectBoundingSphere(Rayd ray);

  /// calculate Axis aligned bounding box from mesh bounding box and current
  /// transforms
  void updateAABB();
};

} // namespace al

#endif
