/*
Allocore Example: Pickable

Description:
This example demonstrates how to associate a mesh with a pickable,
add children pickables and interact with them via the mouse and keyboard.

Author:
Tim Wood, May 2019
*/

#include "al/app/al_App.hpp"

#include "al/graphics/al_Shapes.hpp"

#include "al/math/al_Ray.hpp"
#include "al/ui/al_BoundingBox.hpp"
#include "al/ui/al_Pickable.hpp"

using namespace al;

struct MyApp : public App {
  double t;
  Light light;

  Mesh mesh;

  PickableBB pickable;
  PickableBB child1;
  PickableBB child2;
  PickableBB child3;

  void onCreate() override {
    // position camera, disable mouse to look
    nav().pos(0, 0, 10);
    navControl().useMouse(false);

    // Create a mesh
    addSphere(mesh);
    mesh.generateNormals();

    // Initialize pickableBB with mesh
    pickable.set(mesh);
    pickable.testChildren = false; // disable event propagation, we only enable
                                   // when shift key is down

    // initialize child pickables
    child1.set(mesh);
    child1.scale.set(0.5);
    child1.pose.setPos(Vec3f(2, 0, 0));

    child2.set(mesh);
    child2.scale.set(0.5);
    child2.pose.setPos(Vec3f(2, 0, 0));

    child3.set(mesh);
    child3.scale.set(0.25);
    child3.containedChild = true; // drag events will not allow this child to
                                  // leave its parent's bounding box

    // create pickable heirarchy
    pickable.addChild(child1);
    child1.addChild(child2);
    child2.addChild(child3);
  }

  void onAnimate(double dt) override {
    // move light in a circle
    t += dt;
    light.pos(10 * cos((float)t), 0, 10 * sin((float)t));
  }

  void onDraw(Graphics &g) override {
    // draw lit mesh
    g.clear(0);
    gl::depthTesting(true);
    g.lighting(true);
    g.light(light);

    g.polygonLine(); // as lines

    // pass function to draw pickable and child meshes
    pickable.draw(g, [&](Pickable &p) {
      auto &b = dynamic_cast<PickableBB &>(p);
      if (p.depth == 3) { // change rendering based on pickable depth
        g.polygonFill();
        g.color(1, 0, 0);
      }
      b.drawMesh(g);
    });

    // pass function to draw pickable and children bounding boxes
    g.lighting(false);
    pickable.draw(g, [&](Pickable &p) {
      auto &b = dynamic_cast<PickableBB &>(p);
      b.drawBB(g);
    });
  }

  // helper functions to get scene ray from mouse events
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

  // allow event propagation to children pickables while shift key down
  bool onKeyDown(const Keyboard &k) override {
    pickable.testChildren = k.shift();
    return true;
  }
  bool onKeyUp(const Keyboard &k) override {
    pickable.testChildren = k.shift();
    return true;
  }

  bool onMouseMove(const Mouse &m) override {
    // make a ray from mouse location
    Rayd r = getPickRay(m.x(), m.y());
    pickable.event(PickEvent(Point, r));
    return true;
  }
  bool onMouseDown(const Mouse &m) override {
    Rayd r = getPickRay(m.x(), m.y());
    pickable.event(PickEvent(Pick, r));
    return true;
  }
  bool onMouseDrag(const Mouse &m) override {
    Rayd r = getPickRay(m.x(), m.y());
    pickable.event(PickEvent(Drag, r));
    return true;
  }
  bool onMouseUp(const Mouse &m) override {
    Rayd r = getPickRay(m.x(), m.y());
    pickable.event(PickEvent(Unpick, r));
    return true;
  }
};

int main() {
  MyApp().start();
  return 0;
}
