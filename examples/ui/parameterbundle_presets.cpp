
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterBundle.hpp"

using namespace al;
// This example shows usage of presets applied to parameter bundles
// For this to work you only need to register each bundle instance with the gui

// Create an 'agent' with parameters. All agents have
// the same parameters, but each one can have a different
// value for them
struct Thing {
  Parameter X{"X", "", 0.0f, "", -1.0f, 1.0f};
  Parameter Y{"Y", "", 0.0f, "", -1.0f, 1.0f};
  // The name of the bundle is used to group bundles together
  // i.e. each instance of the bundle is grouped with other instances
  // with the same name
  ParameterBundle bundle{"thing"};

  Thing() {
    // Add shape to mesh
    addCone(mesh);
    mesh.primitive(Mesh::LINE_STRIP);
    // Add parameters to bundle
    bundle << X << Y;
  }

  void draw(Graphics &g) {
    g.pushMatrix();
    // Rotate and color according to bundle index to help differentiate
    g.color(0.4 + 0.2 * bundle.bundleIndex());
    g.translate(X, Y, -8.0);
    g.rotate(30 + 15 * bundle.bundleIndex(), 1.0, 0.0, 0.0);
    g.draw(mesh);
    g.popMatrix();
  }

 private:
  Mesh mesh;
};

struct MyApp : public App {
  void onCreate() override {
    for (int i = 0; i < 3; i++) {
      // Register its parameter bundle with the ControlGUI
      // The three bundles grouped here are displayed as different instances of
      // the "thing", allowing control of one at a time
      gui << things[i].bundle;
      presetHandler << things[i].bundle;
    }
    gui << presetHandler;  // Draw Preset Handler in gui
    navControl().disable();
    gui.init();
    //        presetHandler.verbose();
  }

  void onDraw(Graphics &g) override {
    g.clear();
    for (int i = 0; i < 3; i++) {
      things[i].draw(g);
    }
    gui.draw(g);
  }

 private:
  Thing things[3];
  ControlGUI gui;
  PresetHandler presetHandler;
};

int main() {
  MyApp app;
  app.start();

  return 0;
}
