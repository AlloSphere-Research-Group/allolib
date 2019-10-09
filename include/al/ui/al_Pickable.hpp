
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
  RotatePose,
  Scale
};

struct PickEvent {
  PickEventType type;
  Rayd ray;
  Pose pose;
  float amount;

  PickEvent(PickEventType t, Rayd r) : type(t), ray(r) {}
  PickEvent(PickEventType t, Rayd r, Pose p) : type(t), ray(r), pose(p) {}
  PickEvent(PickEventType t, Rayd r, Vec3f v)
      : type(t), ray(r), pose(v, Quatf()) {}
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
  Parameter scale{"scale", name, 1.0f, "", 0.0f, 10.0f};
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

  Pickable() {
    pose = Pose();
    scaleVec = Vec3f(1);
    hover = false;
    selected = false;
    bundle << hover << selected << pose << scale << scaleVec;
    scale.registerChangeCallback(
        [this](float value) { scaleVec.set(Vec3f(value, value, value)); });
    hover.setHint("hide", 1.0);
    selected.setHint("hide", 1.0);
    scaleVec.setHint(
        "hide",
        1.0);  // We want to show the single value scale by default.
  }

  virtual ~Pickable() {}

  /// intersection test must be implemented
  virtual Hit intersect(Rayd r) = 0;
  virtual Hit intersect(Vec3d v) { return intersect(Rayd(v, Vec3d())); }

  /// override callback
  virtual bool onEvent(PickEvent e, Hit hit) { return false; }

  /// do interaction on self and children, call onEvent callbacks
  virtual bool event(PickEvent e) {
    bool child = false;
    for (unsigned int i = 0; i < children.size(); i++) {
      if (testChildren) {
        Rayd ray = transformRayLocal(e.ray);
        auto ev = PickEvent(e.type, ray);
        child |= children[i]->event(ev);
      } else
        children[i]->clearSelection();
    }
    if (child)
      return true;
    else {
      Hit h = intersect(e.ray);
      return onEvent(e, h);
    }
  }

  void clearSelection() {
    this->foreach ([](Pickable &p) {
      if (p.hover.get()) p.hover = false;
      if (p.selected.get()) p.selected = false;
    });
  }

  bool intersects(Rayd &r) { return intersect(r).hit; }
  bool intersectsChild(Rayd &r) {
    bool child = false;
    Rayd ray = transformRayLocal(r);
    for (unsigned int i = 0; i < children.size(); i++) {
      child |= children[i]->intersectsChild(ray);
      if (child) return child;
    }
    return child;
  }

  Hit intersectChildren(Rayd &r) {
    Hit hmin = Hit(false, r, 1e10, nullptr);
    Rayd ray = transformRayLocal(r);
    for (auto *c : children) {
      Hit h = c->intersect(ray);
      if (h.hit && h.t < hmin.t) {
        hmin = h;
      }
    }
    return hmin;
  }

  void addChild(Pickable &pickable) {
    pickable.parent = this;
    pickable.depth = this->depth + 1;
    children.push_back(&pickable);
  }
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

  void draw(Graphics &g,
            std::function<void(Pickable &p)> f = [](Pickable &p) {}) {
    f(*this);
    pushMatrix(g);
    for (auto *c : children) c->draw(g, f);
    popMatrix(g);
  }

  void foreach (std::function<void(Pickable &p)> pre,
                std::function<void(Pickable &p)> post = [](Pickable &p) {}) {
    pre(*this);
    for (auto *c : children) c->foreach (pre, post);
    post(*this);
  }

  /// transform a ray in world space to local space
  Rayd transformRayLocal(const Rayd &ray) {
    Matrix4d t, r, s;
    Matrix4d model = t.translation(pose.get().pos()) *
                     r.fromQuat(pose.get().quat()) * s.scaling(scaleVec.get());
    Matrix4d invModel = Matrix4d::inverse(model);
    Vec4d o = invModel.transform(Vec4d(ray.o, 1));
    Vec4d d = invModel.transform(Vec4d(ray.d, 0));
    return Rayd(o.sub<3>(0), d.sub<3>(0));
  }

  /// transfrom a vector in local space to world space
  Vec3f transformVecWorld(const Vec3f &v, float w = 1) {
    Matrix4d t, r, s;
    Matrix4d model = t.translation(pose.get().pos()) *
                     r.fromQuat(pose.get().quat()) * s.scaling(scaleVec.get());
    Vec4d o = model.transform(Vec4d(v, w));
    return Vec3f(o.sub<3>(0));
  }
  /// transfrom a vector in world space to local space
  Vec3f transformVecLocal(const Vec3f &v, float w = 1) {
    Matrix4d t, r, s;
    Matrix4d invModel = t.translation(pose.get().pos()) *
                        r.fromQuat(pose.get().quat()) *
                        s.scaling(scaleVec.get());
    Vec4d o = invModel.transform(Vec4d(v, w));
    return Vec3f(o.sub<3>(0));
  }
};

/// Bounding Box PickableMesh
/// @ingroup UI
struct PickableBB : Pickable {
  Mesh *mesh{nullptr};  // pointer to mesh that is wrapped
  BoundingBox bb;       // original bounding box
  BoundingBox aabb;  // axis aligned bounding box (after pose/scale transforms)

  // used for moving pickable naturally
  Vec3f selectPos;
  Vec3f selectOffset;
  Quatf selectQuat;
  float selectDist;

  PickableBB(std::string name = "") {
    Pickable::name = name;
    bundle.name(name);
  }
  PickableBB(Mesh &m) { set(m); }

  /// initialize bounding box;
  void set(Mesh &m) {
    mesh = &m;
    bb.set(*mesh);
  }

  /// override base methods
  Hit intersect(Rayd r) {
    auto ray = transformRayLocal(r);
    double t = intersectBB(ray) * scaleVec.get().x;
    // auto r2 = Rayd(r.o*scaleVec.get(), r.d);
    if (t > 0)
      return Hit(true, r, t, this);
    else
      return Hit(false, r, t, this);
  }
  // bool contains(Vec3d v){ auto p = transformVecLocal(v); return
  // bb.contains(p); }

  bool onEvent(PickEvent e, Hit h) {
    switch (e.type) {
      case Point:
        if (hover.get() != h.hit)
          hover = h.hit;  // setting value propagates via OSC, so only set if
                          // there is a change
        return h.hit;
        break;

      case Pick:
        if (h.hit) {
          prevPose.set(pose.get());
          selectDist = h.t;
          selectPos = h();
          selectOffset = pose.get().pos() - h();  // * scaleVec.get();
        }
        if (selected.get() != h.hit)
          selected = h.hit;  // to avoid triggering change callback if no change

        return h.hit;
        break;

      case PickPose:
        prevPose.set(pose.get());
        selectQuat.set(e.pose.quat());
        selectOffset.set(pose.get().pos() - e.pose.pos());

        if (selected.get() != true) selected = true;

        return true;
        break;

      case Drag:
      case TranslateRay:
        if (selected.get()) {
          Vec3f newPos = h.ray(selectDist) + selectOffset;
          if (parent && (parent->containChildren || containedChild)) {
            auto *p = dynamic_cast<PickableBB *>(parent);
            if (p) {
              // std::cout << "pre: " << newPos << std::endl;

              newPos = min(newPos, p->bb.max - scaleVec.get() * bb.dim / 2);
              // std::cout << "mid: " << newPos << std::endl;

              newPos = max(newPos, p->bb.min + scaleVec.get() * bb.dim / 2);

              // std::cout << "post: " << newPos << std::endl;
              // std::cout << "min: " << p->bb.min << " max: " << p->bb.max << "
              // scale: " << scaleVec.get() << " hdim: " << bb.dim / 2  <<
              // std::endl;
              // newPos -= bb.dim / 2;
            }
          }
          pose = Pose(newPos, pose.get().quat());
          return true;
        } else
          return false;
        break;

      case RotateRay:
        if (selected.get()) {
          Vec3f dir = h.ray(selectDist) - selectPos;
          Quatf q = Quatf().fromEuler(dir.x * 0.5f, -dir.y * 0.5f, 0);

          Vec3f p1 = transformVecWorld(bb.cen);
          pose.setQuat(q * prevPose.quat());
          Vec3f p2 = transformVecWorld(bb.cen);
          pose.setPos(pose.get().pos() + p1 - p2);
          return true;
        } else
          return false;
        break;

      case RotatePose:
        if (selected.get()) {
          Quatf diff =
              e.pose.quat() *
              selectQuat
                  .inverse();  // diff * q0 = q1 --> diff = q1 * q0.inverse

          Vec3f p1 = transformVecWorld(bb.cen);
          pose.setQuat(diff * prevPose.quat());
          Vec3f p2 = transformVecWorld(bb.cen);
          // pose.setPos(e.pose.pos()+selectOffset + p1-p2);
          pose.setPos(pose.get().pos() + p1 - p2);
          return true;
        } else
          return false;
        break;

      case Scale:
        if (selected.get()) {
          Vec3f p1 = transformVecWorld(bb.cen);
          scale = scale + e.amount * 0.01 * scale;
          if (scale < 0.0005) scale = 0.0005;
          // scaleVec.set(scale)
          Vec3f p2 = transformVecWorld(bb.cen);
          pose.setPos(pose.get().pos() + p1 - p2);

          return true;
        } else
          return false;
        break;

      case Unpick:
        if (!hover.get() && selected.get()) selected = false;
        return false;
        break;

      default:
        break;
    }
  }

  // void draw(Graphics &g){
  //   pushMatrix(g);
  //   if(mesh) g.draw(*mesh);

  //   if(selected.get()) g.color(0,1,1);
  //   else if(hover.get()) g.color(1,1,1);
  //   if(selected.get() || hover.get()) bb.draw(g, false);

  //   popMatrix(g);
  // }

  void drawMesh(Graphics &g) {
    if (!mesh) return;
    pushMatrix(g);
    g.draw(*mesh);
    popMatrix(g);
  }
  void drawBB(Graphics &g) {
    if (!selected.get() && !hover.get()) return;
    pushMatrix(g);
    if (selected.get())
      g.color(0, 1, 1);
    else if (hover.get())
      g.color(1, 1, 1);
    bb.draw(g, false);
    popMatrix(g);
  }

  /// set the pickable's center position
  void setCenter(Vec3f &pos) {
    updateAABB();
    Vec3f offset = aabb.cen - pose.get().pos();
    pose = Pose(pos - offset, pose.get().quat());
  }

  /// set pickable's orientation maintaining same center position
  void setQuat(Quatf &q) {
    updateAABB();
    Vec3f cen;
    cen.set(aabb.cen);
    pose = Pose(pose.get().pos(), q);
    setCenter(cen);
  }

  /// intersect ray with pickable BoundingBox
  double intersectBB(Rayd localRay) {
    // Rayd r = transformRayLocal(ray);
    return localRay.intersectBox(bb.cen, bb.dim);
  }

  /// intersect ray with pickable AxisAlignedBoundingBox
  double intersectAABB(Rayd ray) {
    return ray.intersectBox(aabb.cen, aabb.dim);
  }

  /// intersect ray with bounding sphere
  float intersectBoundingSphere(Rayd ray) {
    return ray.intersectSphere(transformVecWorld(bb.cen),
                               (bb.dim * scaleVec.get()).mag() / 2.0);
  }

  /// calculate Axis aligned bounding box from mesh bounding box and current
  /// transforms
  void updateAABB() {
    // thanks to
    // http://zeuxcg.org/2010/10/17/aabb-from-obb-with-component-wise-abs/
    Matrix4d t, r, s;
    Matrix4d model = t.translation(pose.get().pos()) *
                     r.fromQuat(pose.get().quat()) * s.scaling(scaleVec.get());
    Matrix4d absModel(model);
    for (int i = 0; i < 16; i++) absModel[i] = std::abs(absModel[i]);
    Vec4d cen = model.transform(Vec4d(bb.cen, 1));
    Vec4d dim = absModel.transform(Vec4d(bb.dim, 0));
    aabb.setCenterDim(cen.sub<3>(0), dim.sub<3>(0));
  }
};

}  // namespace al

#endif
