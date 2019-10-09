
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"

using namespace al;

// This example shows how to use the ControlGUI class. This class provides
// a quick and simple way to create GUIs from Parameter classes.
// If you need greater control over the GUI layout or properties, use the
// facilites provided by the ParameterGUI class

class MyApp : public App {
 public:
  // Define parameters
  Parameter x{"X", "", 0, "", -2.0, 2.0};
  Parameter y{"Y", "", 0, "", -2.0, 2.0};
  Parameter z{"Z", "", -1.0, "", -4.0, -0.1f};

  ParameterBool show{"Show", "", 1.0};

  ParameterColor color{"Color"};

  ControlGUI
      mControlGUI;  // Control GUI class in charge of generating GUi window

  Mesh mMesh;

  void onInit() override {
    // Add parameters to GUI
    // You can pipe any Parameter type (ParameterColor, ParameterVec3f,
    // ParameterBool, etc.) and the GUI will generate the appropriate controls
    mControlGUI << x << y << z << show << color;

    addSphere(mMesh, 0.01);
    mMesh.primitive(Mesh::LINES);
  }

  void onCreate() override {
    // Always call init() for ControlGUI in onCreate() as it needs a
    // graphics context.
    mControlGUI.init();
    mControlGUI.setTitle("Parameters");
    // You can force the GUI to have a fixed position:
    //    mControlGUI.fixedPosition(true);
  }

  void onDraw(Graphics &g) override {
    // Disable mouse nav to avoid naving while changing gui controls.
    navControl().useMouse(!mControlGUI.usingInput());

    // Draw the "scene"
    g.clear(0);
    if (show == 1.0f) {
      g.pushMatrix();
      g.translate(x, y, z);
      g.color(color);
      g.draw(mMesh);
      g.popMatrix();
    }

    // Draw the control GUI
    mControlGUI.draw(g);
  }
};

int main() {
  MyApp app;
  app.title("Control GUI");
  app.start();
  return 0;
}
