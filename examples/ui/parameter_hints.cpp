
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"

using namespace al;

// This example shows how to use the hints for Parameter

class MyApp : public App {
public:
  // Define parameters
  Parameter x{"X", "", 0, "", -1.0, 1.0};
  Parameter y{"Y", "", 0, "", -1.0, 1.0};
  Parameter z{"Z", "", -2.0, "", -4.0, -0.1f};

  ParameterBool show{"Show Controls", "", 1.0};

  Parameter scale{"Scale", "", 1.0, "", 0.5, 5.0};

  ParameterColor color{"Color"};

  // Control GUI class in charge of generating GUi window
  ControlGUI controlGUI;

  Mesh sphereMesh;

  void onInit() override {
    // Add parameters to GUI
    // You can pipe any Parameter type (ParameterColor, ParameterVec3f,
    // ParameterBool, etc.) and the GUI will generate the appropriate controls
    controlGUI << show << x << y << z << color << scale;

    addSphere(sphereMesh, 0.05);
    sphereMesh.primitive(Mesh::LINES);

    show.registerChangeCallback([&](float value) {
      x.setHint("hide", 1.0 - value);
      y.setHint("hide", 1.0 - value);
      z.setHint("hide", 1.0 - value);
    });

    scale.setHint("input", 2.0);

    // Disable mouse nav to avoid naving while changing gui controls.
    navControl().disable();
  }

  void onCreate() override {
    // Always call init() for ControlGUI in onCreate() as it needs a
    // graphics context.
    controlGUI.init();
    controlGUI.setTitle("Parameter Hints example");
  }

  void onDraw(Graphics &g) override {

    // Draw the "scene"
    g.clear(0);
    g.pushMatrix();
    g.translate(x, y, z);
    g.color(color);
    g.scale(scale);
    g.draw(sphereMesh);
    g.popMatrix();

    // Draw the control GUI
    controlGUI.draw(g);
  }
};

int main() {
  MyApp app;
  app.title("Control GUI");
  app.start();
  return 0;
}
