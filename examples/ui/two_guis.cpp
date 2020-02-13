
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_HtmlInterfaceServer.hpp"
#include "al/ui/al_ParameterMIDI.hpp"

using namespace al;

class MyApp : public App {
 public:
  Parameter x{"X", "", 0, "", -2.0, 2.0};
  Parameter y{"Y", "", 0, "", -2.0, 2.0};
  Parameter z{"Z", "", 0, "", -2.0, 2.0};

  ParameterColor color{"Color"};

  void onCreate() override {
    nav() = Vec3d(0, 0, 2);

    addSphere(mMesh, 0.1);
    mMesh.primitive(Mesh::LINES);

    imguiInit();  // We need to call initIMGUI manually as we want two control
                  // windows
    mPositionGUI.init(5, 5, false);
    mPositionGUI << x << y << z;
    mColorGUI.init(300, 5, false);
    mColorGUI << color;
  }

  void onAnimate(double dt) override {
    navControl().active(!mPositionGUI.usingInput() && !mColorGUI.usingInput());
  }

  void onDraw(Graphics &g) override {
    g.clear(0);

    g.pushMatrix();
    g.translate(x, y, z);
    g.color(color);
    g.draw(mMesh);
    g.popMatrix();

    imguiBeginFrame();
    mPositionGUI.draw(g);
    mColorGUI.draw(g);
    imguiEndFrame();
    imguiDraw();
  }

 private:
  ControlGUI mPositionGUI;
  ControlGUI mColorGUI;

  Mesh mMesh;
};

int main() {
  MyApp app;
  app.dimensions(800, 600);
  app.title("Presets GUI");
  app.fps(30);
  app.start();
  return 0;
}
