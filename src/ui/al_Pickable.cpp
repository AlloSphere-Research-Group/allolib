#include "al/ui/al_Pickable.hpp"

using namespace al;

Pickable::Pickable() {
  pose = Pose();
  scaleVec = Vec3f(1);
  hover = false;
  selected = false;
  bundle << hover << selected << pose << scale << scaleVec;
  scale.registerChangeCallback(
      [this](float value) { scaleVec.set(Vec3f(value, value, value)); });
  hover.setHint("hide", 1.0);
  selected.setHint("hide", 1.0);
  scaleVec.setHint("hide",
                   1.0);  // We want to show the single value scale by default.
}

bool Pickable::event(PickEvent e) {
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

void Pickable::clearSelection() {
  this->foreach ([](Pickable &p) {
    if (p.hover.get()) p.hover = false;
    if (p.selected.get()) p.selected = false;
  });
}

bool Pickable::intersectsChild(Rayd &r) {
  bool child = false;
  Rayd ray = transformRayLocal(r);
  for (unsigned int i = 0; i < children.size(); i++) {
    child |= children[i]->intersectsChild(ray);
    if (child) return child;
  }
  return child;
}

Hit Pickable::intersectChildren(Rayd &r) {
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

void Pickable::addChild(Pickable &pickable) {
  pickable.parent = this;
  pickable.depth = this->depth + 1;
  children.push_back(&pickable);
}

void Pickable::draw(Graphics &g, std::function<void(Pickable &)> f) {
  f(*this);
  pushMatrix(g);
  for (auto *c : children) c->draw(g, f);
  popMatrix(g);
}

Rayd Pickable::transformRayLocal(const Rayd &ray) {
  Matrix4d t, r, s;
  Matrix4d model = t.translation(pose.get().pos()) *
                   r.fromQuat(pose.get().quat()) * s.scaling(scaleVec.get());
  Matrix4d invModel = Matrix4d::inverse(model);
  Vec4d o = invModel.transform(Vec4d(ray.o, 1));
  Vec4d d = invModel.transform(Vec4d(ray.d, 0));
  return Rayd(o.sub<3>(0), d.sub<3>(0));
}

Vec3f Pickable::transformVecWorld(const Vec3f &v, float w) {
  Matrix4d t, r, s;
  Matrix4d model = t.translation(pose.get().pos()) *
                   r.fromQuat(pose.get().quat()) * s.scaling(scaleVec.get());
  Vec4d o = model.transform(Vec4d(v, w));
  return Vec3f(o.sub<3>(0));
}

Vec3f Pickable::transformVecLocal(const Vec3f &v, float w) {
  Matrix4d t, r, s;
  Matrix4d invModel = t.translation(pose.get().pos()) *
                      r.fromQuat(pose.get().quat()) * s.scaling(scaleVec.get());
  Vec4d o = invModel.transform(Vec4d(v, w));
  return Vec3f(o.sub<3>(0));
}

void PickableBB::set(Mesh &m) {
  mesh = &m;
  bb.set(*mesh);
}

Hit PickableBB::intersect(Rayd r) {
  auto ray = transformRayLocal(r);
  double t = intersectBB(ray) * scaleVec.get().x;
  // auto r2 = Rayd(r.o*scaleVec.get(), r.d);
  if (t > 0)
    return Hit(true, r, t, this);
  else
    return Hit(false, r, t, this);
}

bool PickableBB::onEvent(PickEvent e, Hit h) {
  switch (e.type) {
    case Point:
      if (hover.get() != h.hit)
        hover = h.hit;  // setting value propagates via OSC, so only set if
      // there is a change
      return h.hit;

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

    case PickPose:
      prevPose.set(pose.get());
      selectQuat.set(e.pose.quat());
      selectOffset.set(pose.get().pos() - e.pose.pos());

      if (selected.get() != true) selected = true;

      return true;

    case Drag:
    case TranslateRay:
      if (selected.get() == 1.0f) {
        Vec3f newPos = h.ray(selectDist) + selectOffset;
        if (parent && (parent->containChildren || containedChild)) {
          auto *p = dynamic_cast<PickableBB *>(parent);
          if (p) {
            // std::cout << "pre: " << newPos << std::endl;

            newPos = min(newPos, p->bb.max - bb.max);
            // std::cout << "mid: " << newPos << std::endl;

            newPos = max(newPos, p->bb.min - bb.min);

            // std::cout << "post: " << newPos << std::endl;
            // std::cout << "pmin: " << p->bb.min << " pmax: " << p->bb.max << std::endl;
            // std::cout << "min: " << bb.min << " max: " << bb.max << std::endl;
          }
        }
        pose = Pose(newPos, pose.get().quat());
        return true;
      } else
        return false;

    case RotateRayTrackball:
      if (selected.get() == 1.0f) {        
        Vec3f p = pose.get().pos();
        Vec3f v1 = (selectPos - p).normalize(); 
        Vec3f v2 = (h.ray(selectDist) - p).normalize(); 
        Vec3f n = v1.cross(v2); // axis of rotation
        double angle = acos(v1.dot(v2));
        Quatf qf = Quatf().fromAxisAngle(angle, n).normalize();
          
        Vec3f p1 = transformVecWorld(bb.cen);
        pose.setQuat(qf * prevPose.quat());
        Vec3f p2 = transformVecWorld(bb.cen);
        pose.setPos(pose.get().pos() + p1 - p2);
        return true;
      } else
        return false;

    case RotateRay:
      if (selected.get() == 1.0f) {
        float dist = (h.ray.o - pose.get().pos()).mag();
        float amt = dist / 10 * (-10) + 10; // 0 - 10+ -> 10 - 0.1
        if(amt < 0.1) amt = 0.1;
        Vec3f dir = h.ray(selectDist) - selectPos;
        Quatf q = Quatf().fromEuler(amt * dir.x, amt * -dir.y, 0);
        Vec3f p1 = transformVecWorld(bb.cen);
        pose.setQuat(q * prevPose.quat());
        Vec3f p2 = transformVecWorld(bb.cen);
        pose.setPos(pose.get().pos() + p1 - p2);
        return true;
      } else
        return false;

    case RotateTurntable:
      if (selected.get() == 1.0f) {
        float amt = 0.005;
       
        // Quatf qy = Quatf().fromAxisAngle(amt*e.vec.x, Vec3f(0,1,0)).normalize();
        Quatf qy = Quatf().fromAxisAngle(amt*e.vec.x, e.pose.quat().toVectorY()).normalize();
        Quatf qx = Quatf().fromAxisAngle(amt*e.vec.y, e.pose.quat().toVectorX()).normalize();
        // Quatf q = Quatf().fromEuler(amt * e.vec.x, amt * e.vec.y, 0);

        Vec3f p1 = transformVecWorld(bb.cen);
        // pose.setQuat(q * prevPose.quat());
        // pose.setQuat(qy * qx * prevPose.quat());
        pose.setQuat(qx * prevPose.quat() * qy);
        Vec3f p2 = transformVecWorld(bb.cen);
        pose.setPos(pose.get().pos() + p1 - p2);
        return true;
      } else
        return false;


    case RotatePose:
      if (selected.get() == 1.0f) {
        Quatf diff =
            e.pose.quat() *
            selectQuat.inverse();  // diff * q0 = q1 --> diff = q1 * q0.inverse

        Vec3f p1 = transformVecWorld(bb.cen);
        pose.setQuat(diff * prevPose.quat());
        Vec3f p2 = transformVecWorld(bb.cen);
        // pose.setPos(e.pose.pos()+selectOffset + p1-p2);
        pose.setPos(pose.get().pos() + p1 - p2);
        return true;
      } else
        return false;

    case Scale:
      if (selected.get() == 1.0f) {
        Vec3f p1 = transformVecWorld(bb.cen);
        scale = scale + e.amount * 0.01 * scale;
        if (scale < 0.0005f) scale = 0.0005f;
        // scaleVec.set(scale)
        Vec3f p2 = transformVecWorld(bb.cen);
        pose.setPos(pose.get().pos() + p1 - p2);

        return true;
      } else
        return false;

    case Unpick:
      if (hover.get() != 1.0f && selected.get() == 1.0f) selected = false;
      return false;
  }
  return false;
}

void PickableBB::drawMesh(Graphics &g) {
  if (!mesh) return;
  pushMatrix(g);
  g.draw(*mesh);
  popMatrix(g);
}

void PickableBB::drawBB(Graphics &g) {
  if (!selected.get() && !hover.get()) return;
  pushMatrix(g);
  if (selected.get())
    g.color(0, 1, 1);
  else if (hover.get())
    g.color(1, 1, 1);
  bb.draw(g, false);
  popMatrix(g);
}

void PickableBB::setCenter(Vec3f &pos) {
  updateAABB();
  Vec3f offset = aabb.cen - pose.get().pos();
  pose = Pose(pos - offset, pose.get().quat());
}

void PickableBB::setQuat(Quatf &q) {
  updateAABB();
  Vec3f cen;
  cen.set(aabb.cen);
  pose = Pose(pose.get().pos(), q);
  setCenter(cen);
}

double PickableBB::intersectBB(Rayd localRay) {
  // Rayd r = transformRayLocal(ray);
  return localRay.intersectBox(bb.cen, bb.dim);
}

double PickableBB::intersectAABB(Rayd ray) {
  return ray.intersectBox(aabb.cen, aabb.dim);
}

float PickableBB::intersectBoundingSphere(Rayd ray) {
  return ray.intersectSphere(transformVecWorld(bb.cen),
                             (bb.dim * scaleVec.get()).mag() / 2.0);
}

void PickableBB::updateAABB() {
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
