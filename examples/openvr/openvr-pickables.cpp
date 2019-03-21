#include "al/core.hpp"
#include "module/openvr/al_OpenVRWrapper.hpp"

using namespace al;

struct MyApp : public App {

  OpenVRWrapper mOpenVR;
  PickableManager mPickableManager;
  VAOMesh mSphere;
  VAOMesh mBox;

  void onCreate() override {
    // Initialize openVR in onCreate. A graphics context is needed.
    if (!mOpenVR.init()) std::cerr << "ERROR: OpenVR init returned error" << std::endl;

    addSphere(mSphere, 0.1f, 8, 8);
    mSphere.primitive(Mesh::LINE_STRIP);
    mSphere.update();
    addWireBox(mBox, 0.1f);
    mBox.scale(1, 0.1, 1);
    mBox.update();

    // create some pickables
    for (int i = 0; i < 9; i++){
      Pickable *p = new Pickable;
      p->set(mSphere);
      p->pose = Pose(Vec3f(i * 0.25f - 1, 0, 0), Quatf());

      Pickable *child = new Pickable;
      child->set(mBox);
      p->addChild((*child));
      mPickableManager << p;
    }
  }

  void onAnimate(double dt) override {
    // Update traking and controller data;
    mOpenVR.update();

    //openVR draw.
    // Draw in onAnimate, to make sure drawing happens only once per frame
    // Pass a function that takes Graphics &g argument
    mOpenVR.draw(std::bind(&MyApp::drawScene, this, std::placeholders::_1), mGraphics);
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
    // Draw markers for the controllers
    // The openVR object is available in the VRRenderer class to query the controllers
    g.pushMatrix();
    g.translate(mOpenVR.LeftController.pos);
    g.rotate(mOpenVR.LeftController.quat);
    g.scale(0.1);
    g.color(1);
    g.polygonMode(Graphics::LINE);
    g.draw(mCube);
    g.popMatrix();

    //right hand
    g.pushMatrix();
    g.translate(mOpenVR.RightController.pos);
    // std::cout << openVR->RightController.pos.x << openVR->RightController.pos.y << openVR->RightController.pos.z << std::endl;
    g.rotate(mOpenVR.RightController.quat);
    g.scale(0.1);
    g.color(1);
    g.polygonMode(Graphics::LINE);
    g.draw(mCube);
    g.popMatrix();
  }

  void onDraw(Graphics &g) override {
    // This onDraw determines what is seen on the Desktop window.
    // In this case, the desktop nav is independent to the HMD nav
    drawScene(g);
  }

  void onExit() override {
    mOpenVR.close();
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
