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
      Pickable *p = new Pickable;
      p->set(mSphere);
      p->pose = Pose(Vec3f(i*0.25f - 1,0,0),Quatf());
      
      Pickable *child = new Pickable;
      child->set(mBox);
      p->addChild((*child));
      mPickableManager << p;
    }
  }

  void onAnimate(double dt) override {
    // std::cout << "min  " << ((Pickable*)mPickableManager.pickables()[0]->children[0])->bb.min << std::endl;
    // std::cout << "pmin " << ((Pickable*)mPickableManager.pickables()[0])->bb.min << std::endl;
    // std::cout << mPickableManager.pickables()[0]->children[0]->pose.get().pos() << std::endl;
  }

  void onDraw(Graphics &g) override {

    g.clear();
    g.depthTesting(true);

    for(auto p : mPickableManager.pickables()){
      g.color(1,1,1);
      p->draw(g);
      g.color(1,0,0);
      p->drawChildren(g);
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
