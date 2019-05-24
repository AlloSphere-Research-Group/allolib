#include "al/core.hpp"
#include "al/util/ui/al_Pickable.hpp"
#include "al/util/ui/al_PickableManager.hpp"

using namespace al;

struct MyApp : public App {

  PickableManager mPickableManager;
  VAOMesh mSphere;
  VAOMesh mBox;

  void onCreate() override {

    nav().pos(0,0,3);
    navControl().useMouse(false);

    addSphere(mSphere, 0.1f, 8, 8);
    mSphere.primitive(Mesh::LINE_STRIP);
    mSphere.update();
    addWireBox(mBox, 0.1f);
    mBox.scale(1,0.1,1);
    mBox.update();

    // create some pickables
    for (int i = 0; i < 9; i++){
      PickableBB *p = new PickableBB;
      p->set(mSphere);
      p->pose = Pose(Vec3f(i*0.25f - 1,0,0),Quatf());
      p->containChildren = true;
      
      PickableBB *child = new PickableBB;
      child->set(mBox);
      p->addChild(child);
      mPickableManager += p;
    }
  }

  void onAnimate(double dt) override {}

  void onDraw(Graphics &g) override {

    g.clear();
    g.depthTesting(true);

    for(auto pickable : mPickableManager.pickables()){
      g.color(1,1,1);

      // pass function to draw pickable and child meshes
      pickable->draw(g, [&](Pickable &p){
        auto &b = dynamic_cast<PickableBB&>(p);
        if(p.depth == 1) g.color(1,0,0);
        b.drawMesh(g);
        b.drawBB(g);
      });
    }


  }

  virtual void onMouseMove(const Mouse &m){
    mPickableManager.onMouseMove(graphics(), m, width(), height());
  }
  virtual void onMouseDown(const Mouse &m){
    mPickableManager.onMouseDown(graphics(), m, width(), height());
  }
  virtual void onMouseDrag(const Mouse &m){
    mPickableManager.onMouseDrag(graphics(), m, width(), height());
  }
  virtual void onMouseUp(const Mouse &m){
    mPickableManager.onMouseUp(graphics(), m, width(), height());
  }
};

int main(int argc, char *argv[]){
  MyApp app;

  app.start();
  return 0;
}
