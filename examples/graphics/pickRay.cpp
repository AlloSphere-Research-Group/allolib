/*
Allocore Example: pickRay

Description:
The example demonstrates how to interact with objects using ray intersection
tests.

Author:
Tim Wood, 9/19/2014
*/

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Ray.hpp"

using namespace al;

#define N 10

struct PickRayDemo : App {
  Material material;
  Light light;
  Mesh m;

  Vec3f pos[N];
  Vec3f offset[N];  // difference from intersection to center of sphere
  float dist[N];    // distance of intersection
  bool hover[N];    // mouse is hovering over sphere
  bool selected[N]; // mouse is down over sphere

  void onCreate() override {
    nav().pos(0, 0, 10);
    light.pos(0, 0, 10);
    addSphere(m, 0.5);
    m.generateNormals();

    for (int i = 0; i < N; i++) {
      pos[i] = Vec3f(rnd::uniformS(), rnd::uniformS(), 0.f) * 2.f;
      offset[i] = Vec3f();
      dist[i] = 0.f;
      hover[i] = false;
      selected[i] = false;
    }

    // disabel nav control mouse drag to look
    navControl().useMouse(false);
  }

  virtual void onDraw(Graphics &g) override {
    g.clear(0);
    gl::depthTesting(true);
    g.lighting(true);

    for (int i = 0; i < N; i++) {
      g.pushMatrix();
      g.translate(pos[i]);
      if (selected[i])
        g.color(1, 0, 1);
      else if (hover[i])
        g.color(0, 1, 1);
      else
        g.color(1, 1, 1);
      g.draw(m);
      g.popMatrix();
    }
  }

  Vec3d unproject(Vec3d screenPos) {
    auto &g = graphics();
    auto mvp = g.projMatrix() * g.viewMatrix() * g.modelMatrix();
    Matrix4d invprojview = Matrix4d::inverse(mvp);
    Vec4d worldPos4 = invprojview.transform(screenPos);
    return worldPos4.sub<3>(0) / worldPos4.w;
  }

  Rayd getPickRay(int screenX, int screenY) {
    Rayd r;
    Vec3d screenPos;
    screenPos.x = (screenX * 1. / width()) * 2. - 1.;
    screenPos.y = ((height() - screenY) * 1. / height()) * 2. - 1.;
    screenPos.z = -1.;
    Vec3d worldPos = unproject(screenPos);
    r.origin().set(worldPos);

    screenPos.z = 1.;
    worldPos = unproject(screenPos);
    r.direction().set(worldPos);
    r.direction() -= r.origin();
    r.direction().normalize();
    return r;
  }

  bool onMouseMove(const Mouse &m) override {
    // make a ray from mouse location
    Rayd r = getPickRay(m.x(), m.y());

    // intersect ray with each sphere in scene
    for (int i = 0; i < N; i++) {
      // intersect sphere at center pos[i] and radius 0.5f
      // returns the distance of the intersection otherwise -1
      float t = r.intersectSphere(pos[i], 0.5f);
      hover[i] = t > 0.f;
    }
    return true;
  }
  bool onMouseDown(const Mouse &m) override {
    Rayd r = getPickRay(m.x(), m.y());

    for (int i = 0; i < N; i++) {
      float t = r.intersectSphere(pos[i], 0.5f);
      selected[i] = t > 0.f;

      // if intersection occured store and offset and distance for moving the
      // sphere
      if (t > 0.f) {
        offset[i] = pos[i] - r(t);
        dist[i] = t;
      }
    }
    return true;
  }
  bool onMouseDrag(const Mouse &m) override {
    Rayd r = getPickRay(m.x(), m.y());

    for (int i = 0; i < N; i++) {
      // if sphere previously selected move sphere
      if (selected[i]) {
        Vec3f newPos = r(dist[i]) + offset[i];
        pos[i].set(newPos);
      }
    }
    return true;
  }
  bool onMouseUp(const Mouse &m) override {
    // deselect all spheres
    for (int i = 0; i < N; i++)
      selected[i] = false;
    return true;
  }
};
int main() {
  PickRayDemo app;
  app.start();
  return 0;
}
