/*
Allocore Example: PickableRotateHandle

Description:
This example shows how to add a rotation handle as a child 
to interact with its parent pickable

Author:
Tim Wood, April 2016
*/

#include "al/core.hpp"
#include "al/util/al_Ray.hpp"
#include "al/util/ui/al_Pickable.hpp"
#include "al/util/ui/al_PickableRotateHandle.hpp"


using namespace al;

class MyApp : public App {
public:

  Material material; 
  
  Mesh mesh; 

  Pickable pickable;
  // TranslateHandle th;
  PickableRotateHandle rh;

  void onCreate(){

    // position camera, disable mouse to look
    nav().pos(0,0,10);
    navControl().useMouse(false);

    // Create a torus mesh
    addTorus(mesh);
    mesh.scale(0.5);
    mesh.generateNormals();

    // Initialize Pickable
    pickable.set(mesh);
    
    // add translate handle as child
    pickable.addChild(rh);
    // pickable.addChild(th);
  }

  void onDraw(Graphics& g){

    g.clear(0);

    // draw lit mesh
    g.lighting(true);
    // g.light(light);
    g.color(1,1,1);
    pickable.drawMesh(g);
    
    g.lighting(false);
    pickable.drawBB(g);

    g.depthTesting(false);
    pickable.drawChildren(g);
    g.depthTesting(true);
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
