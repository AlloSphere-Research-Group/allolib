#include "al/core.hpp"

#include "al/util/ui/al_Pickable.hpp"
#include "al/util/ui/al_PickableManager.hpp"

#ifdef BUILD_VR
#include "module/openvr/al_OpenVRWrapper.hpp"
#endif

using namespace al;

struct MyApp : public App {

#ifdef BUILD_VR
  OpenVRWrapper mOpenVR;
#endif
  PickableManager mPickableManager;
  VAOMesh mSphere;
  VAOMesh mBox;
  VAOMesh mController;
  Mesh mRay;
  Mesh mHit;

  void onCreate() override {
#ifdef BUILD_VR
    // Initialize openVR in onCreate. A graphics context is needed.
    if (!mOpenVR.init()) std::cerr << "ERROR: OpenVR init returned error" << std::endl;
#else
    std::cerr << "Not built with OpenVR support." << std::endl;
#endif

    nav().pos(0,0,3);
    navControl().useMouse(false);

    addSphere(mSphere, 0.1f, 8, 8);
    mSphere.primitive(Mesh::LINE_STRIP);
    mSphere.update();
    addWireBox(mBox, 0.1f);
    mBox.scale(1, 0.1, 1);
    mBox.update();
    addWireBox(mController, 0.1f);
    mController.scale(0.1, 0.1, 1);
    mController.update();
    addSphere(mHit, 0.01f, 8, 8);
    mRay.primitive(Mesh::LINES);

    // create some pickables
    for (int i = 0; i < 9; i++){
      Pickable *p = new Pickable;
      p->set(mSphere);
      p->pose = Pose(Vec3f(i * 0.25f - 1, 0, 0), Quatf());
      if(i == 0) p->scale = 0.5f;
      if(i == 4) p->pose = Pose(Vec3f(i * 0.25f - 1, 0, 0), Quatf().fromEuler(Vec3f(0.5,0,0)));
      if(i == 8) p->scale = 2.0f;

      Pickable *child = new Pickable;
      child->set(mBox);
      p->addChild((*child));
      mPickableManager << p;
    }
  }

  void onAnimate(double dt) override {
#ifdef BUILD_VR
    // Update traking and controller data;
    mOpenVR.update();

    // OpenVR Pickable events
    auto l = mOpenVR.LeftController;
    auto r = mOpenVR.RightController;

    auto ray = r.ray();

    if(r.triggerPress()){
      mPickableManager.pick(ray);
    } else if(r.gripPress()){
      mPickableManager.pick(ray);
      mPickableManager.grab(r.pose());
    } else if(r.triggerDown()){
      mPickableManager.drag(ray, r.vel);
    } else if(r.gripDown()){
      mPickableManager.rotate(r.pose());
    } else if(r.triggerRelease() || r.gripRelease()){
      mPickableManager.unpick(ray);
    } else {
      mPickableManager.point(ray);
    }
    
    // if(r.gripPress()){
    //   mPickableManager.pick(r.pos);
    // } else if(r.gripDown()){
    //   if(l.gripDown()){

    //   }else{
    //     mPickableManager.drag(r.pos, r.vel);
    //   }
    // } else if(r.gripRelease()){
    //   mPickableManager.unpick(r.pos);
    // } else {
    //   mPickableManager.point(r.pos);
    // }

    //openVR draw.
    // Draw in onAnimate, to make sure drawing happens only once per frame
    // Pass a function that takes Graphics &g argument
    mOpenVR.draw(std::bind(&MyApp::drawScene, this, std::placeholders::_1), mGraphics);
#endif
  }

  void drawScene(Graphics &g){
    g.clear();
    g.depthTesting(true);

    for (auto p : mPickableManager.pickables()){
      g.color(1, 1, 1);
      p->draw(g);
      g.color(1, 0, 0);
      p->drawChildren(g);
    }

    auto h1 = mPickableManager.lastPick();
    if(h1.hit){
      g.pushMatrix();
      g.translate(h1());
      g.color(0,0,1);
      g.draw(mHit);
      g.popMatrix();
    }
    auto h2 = mPickableManager.lastPoint();
    if(h2.hit){
      g.pushMatrix();
      g.translate(h2());
      g.color(0,1,0);
      g.draw(mHit);
      g.popMatrix();
    }

    // Draw markers for the controllers
    // The openVR object is available in the VRRenderer class to query the controllers
    g.pushMatrix();
#ifdef BUILD_VR
    g.translate(mOpenVR.LeftController.pos);
    g.rotate(mOpenVR.LeftController.quat);
#endif
    g.color(0,1,1);
    g.draw(mController);
    g.popMatrix();

    //right hand
    g.pushMatrix();
#ifdef BUILD_VR
    g.translate(mOpenVR.RightController.pos);
    g.rotate(mOpenVR.RightController.quat);
#endif
    g.color(1,0,1);
    g.draw(mController);
    g.popMatrix();

    //draw controller rays
#ifdef BUILD_VR
    auto r1 = mOpenVR.RightController.ray();
    auto r2 = mOpenVR.LeftController.ray();
    mRay.reset();
    mRay.vertices().push_back(r1.o);
    mRay.vertices().push_back(r1.o + r1.d*5);
    mRay.vertices().push_back(r2.o);
    mRay.vertices().push_back(r2.o + r2.d*5);
    g.draw(mRay);
#endif
  }

  void onDraw(Graphics &g) override {
    // This onDraw determines what is seen on the Desktop window.
    // In this case, the desktop nav is independent to the HMD nav
    drawScene(g);
  }

  void onExit() override {
#ifdef BUILD_VR
    mOpenVR.close();
#endif
  }

  virtual void onMouseMove(const Mouse &m) override {
    mPickableManager.onMouseMove(graphics(), m, width(), height());
  }
  virtual void onMouseDown(const Mouse &m) override {
    mPickableManager.onMouseDown(graphics(), m, width(), height());
  }
  virtual void onMouseDrag(const Mouse &m) override {
    mPickableManager.onMouseDrag(graphics(), m, width(), height());
  }
  virtual void onMouseUp(const Mouse &m) override {
    mPickableManager.onMouseUp(graphics(), m, width(), height());
  }
};


int main(int argc, char *argv[]){
  MyApp app;

  app.start();
  return 0;
}
