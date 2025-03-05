
#ifndef __AL_ROTATE_HANDLE_HPP__
#define __AL_ROTATE_HANDLE_HPP__

#include <al/ui/al_Pickable.hpp>
#include <cfloat>

namespace al {

/// PickableRotateHandle
/// @ingroup UI
struct PickableRotateHandle : Pickable {
  Vec3f downPos, newPos;
  Vec3f downDir, newDir;
  Quatf rotate;
  bool hover[3] = {false, false, false};
  bool selected[3] = {false, false, false};

  float size = 1.0f;
  float dr = 0.1f;

  PickableRotateHandle() { rotate = Quatf::identity(); }

  void addCircle(Mesh &m, float r, int n) {
    double inc = M_2PI / n;
    double phase = 0.0;
    for (int i = 0; i <= n; i++) {
      float x = cos(phase) * r;
      float y = sin(phase) * r;
      m.vertex(x, y, 0);
      phase += inc;
    }
  }

  void set(PickableRotateHandle &rh) {
    downPos.set(rh.downPos);
    downDir.set(rh.downDir);
    newPos.set(rh.newPos);
    newDir.set(rh.newDir);
    rotate.set(rh.rotate);
    for (int i = 0; i < 3; i++) {
      hover[i] = rh.hover[i];
      selected[i] = rh.selected[i];
    }
  }

  void draw(Graphics &g) {
    //    glPushAttrib(GL_ALL_ATTRIB_BITS);
    Mesh m;
    m.primitive(Mesh::LINE_STRIP);

    Quatf q;
    for (int i = 0; i < 3; i++) {
      m.reset();
      addCircle(m, size, 30);
      g.pushMatrix();
      g.translate(pose.get().pos());
      g.color(i == 0, i == 1, i == 2);
      switch (i) {
        case 0:
          q.fromEuler(M_PI / 2, 0, 0);
          break;
        case 1:
          q.fromEuler(0, -M_PI / 2, 0);
          break;
        case 2:
          q.fromEuler(0, 0, 0);
          break;
      }
      g.rotate(q);
      if (hover[i])
        addCircle(m, size - dr,
                  30);  //{ m.ribbonize(.03f);
                        // m.primitive(Mesh::TRIANGLE_STRIP);}//g.lineWidth(3);
      // else g.lineWidth(1);
      g.draw(m);
      g.popMatrix();
    }

    for (int i = 0; i < 3; i++) {
      if (selected[i]) {
        m.reset();
        m.primitive(Mesh::LINES);
        m.vertex(pose.get().pos());
        m.vertex(pose.get().pos() + newDir.normalized() * (size));
        m.vertex(pose.get().pos());
        m.vertex(pose.get().pos() + downDir.normalized() * (size));
        g.color(i == 0, i == 1, i == 2);
        g.draw(m);
      }
    }
    //    glPopAttrib();
  }

  Hit intersect(Rayd r) override {
    double t = r.intersectSphere(pose.get().pos(), size);
    if (t > 0)
      return Hit(true, r, t, this);
    else
      return Hit(false, r, t, this);
  }

  bool onEvent(PickEvent e, Hit h) override {
    switch (e.type) {
      case Point:
        if (h.ray.intersectsSphere(pose.get().pos(), size)) {
          float t = -1;
          float min = FLT_MAX;
          int minIdx = -1;
          for (int i = 0; i < 3; i++) {
            hover[i] = false;
            t = h.ray.intersectCircle(pose.get().pos(),
                                      Vec3f(i == 0, i == 1, i == 2), size,
                                      size - dr);
            if (t > 0 && t < min) {
              min = t;
              minIdx = i;
            }
          }
          if (minIdx >= 0) {
            hover[minIdx] = true;
            return true;
          }
        } else
          for (int i = 0; i < 3; i++) hover[i] = false;
        return false;
        break;

      case Pick:
        if (h.ray.intersectsSphere(pose.get().pos(), size)) {
          float t = -1;
          float min = FLT_MAX;
          int minIdx = -1;
          for (int i = 0; i < 3; i++) {
            selected[i] = false;
            t = h.ray.intersectCircle(pose.get().pos(),
                                      Vec3f(i == 0, i == 1, i == 2), size,
                                      size - dr);
            if (t > 0 && t < min) {
              min = t;
              minIdx = i;
            }
          }
          if (minIdx >= 0) {
            selected[minIdx] = true;
            downDir.set(h.ray(min) - pose.get().pos());
            newDir.set(h.ray(min) - pose.get().pos());
            if (parent) {
              parent->prevPose.set(parent->pose.get());
            }
            return true;
          }
        }
        return false;
        break;

      case Drag:
        for (int i = 0; i < 3; i++) {
          if (selected[i]) {
            float t = h.ray.intersectPlane(pose.get().pos(),
                                           Vec3f(i == 0, i == 1, i == 2));
            if (t > 0) {
              newDir.set(h.ray(t) - pose.get().pos());
              rotate = Quatf::getRotationTo(downDir.normalized(),
                                            newDir.normalized());
              if (parent) {
                // rotate parent around rotation handle offset, probably a
                // better way to do this :p
                Vec3f p1 = parent->transformVecWorld(pose.get().pos());
                parent->pose.setQuat(parent->pose.get().quat() * rotate);
                Vec3f p2 = parent->transformVecWorld(pose.get().pos());
                parent->pose.setPos(parent->pose.get().pos() + p1 - p2);
              }
              return true;
            }
          }
        }
        return false;
        break;

      case Unpick:
        for (int i = 0; i < 3; i++) selected[i] = false;
        return false;
        break;

      default:
        break;
    }
    return false;
  }
};

}  // namespace al

#endif
