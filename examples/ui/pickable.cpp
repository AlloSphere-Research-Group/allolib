/*
Allocore Example: Pickable

Description:
This example demonstrates how associate a mesh with a pickable
and interact with it via the mouse

Author:
Tim Wood, April 2016
*/

#include "al/core.hpp"
#include "al/util/al_Ray.hpp"
#include "al/util/ui/al_BoundingBox.hpp"
#include "al/util/ui/al_Pickable.hpp"

using namespace al;

class MyApp : public App {
public:

  bool loadedFont;
  double t;
  Light light;      
  
  Mesh mesh; 

  Pickable pickable;

  void onCreate(){

    // position camera, disable mouse to look
    nav().pos(0,0,20);
    navControl().useMouse(false);

    // Create a red spheres mesh
    addSphere(mesh,1);
    mesh.translate(-3,3,0);
    addSphere(mesh,2);
    mesh.translate(-2.7,2.7,0);
    addSphere(mesh,0.7);
    mesh.translate(2.7,-2.7,0);

    mesh.generateNormals();
    mesh.color(RGB(1,0,0));

    // Initialize BoundingBox
    pickable.set(mesh);

  }

  void onAnimate(double dt){
    // move light in a circle
    t += dt;
    light.pos(10*cos(t),0,10*sin(t));
  }

  void onDraw(Graphics& g){

    // draw lit mesh
    g.clear(0);
    g.depthTesting(true);
    g.lighting(true);
    g.light(light);

    pickable.drawMesh(g);
    
    g.lighting(false);
    pickable.drawBB(g);
  }

  Vec3d unproject(Vec3d screenPos){
    auto& g = graphics();
    auto mvp = g.projMatrix() * g.viewMatrix() * g.modelMatrix();
    Matrix4d invprojview = Matrix4d::inverse(mvp);
    Vec4d worldPos4 = invprojview.transform(screenPos);
    return worldPos4.sub<3>(0) / worldPos4.w;
  }

  Rayd getPickRay(int screenX, int screenY){
    Rayd r;
    Vec3d screenPos;
    screenPos.x = (screenX*1. / width()) * 2. - 1.;
    screenPos.y = ((height() - screenY)*1. / height()) * 2. - 1.;
    screenPos.z = -1.;
    Vec3d worldPos = unproject(screenPos);
    r.origin().set(worldPos);

    screenPos.z = 1.;
    worldPos = unproject(screenPos);
    r.direction().set( worldPos );
    r.direction() -= r.origin();
    r.direction().normalize();
    return r;
  }

  virtual void onMouseMove(const Mouse& m){
    // make a ray from mouse location
    Rayd r = getPickRay(m.x(), m.y());
    pickable.point(r);
  }
  virtual void onMouseDown(const Mouse& m){
    Rayd r = getPickRay(m.x(), m.y());
    pickable.pick(r);
  }
  virtual void onMouseDrag(const Mouse& m){
    Rayd r = getPickRay(m.x(), m.y());
    pickable.drag(r);
  }
  virtual void onMouseUp(const Mouse& m){
    Rayd r = getPickRay(m.x(), m.y());
    pickable.unpick(r);
  }
};

int main(){
  MyApp().start();
}
