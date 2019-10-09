
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"

#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterBundle.hpp"

using namespace al;

struct Thing {
  Parameter X{"X", "", 0.0f, "", -1.0f, 1.0f};
  Parameter Y{"Y", "", 0.0f, "", -1.0f, 1.0f};
  Parameter Rotation{"Rotation", "", 0.0f, "", -90.f, 90.0f};
  ParameterBundle bundle{"thing"};

  Thing() {
    // Add shape to mesh
    addCone(mesh);
    mesh.primitive(Mesh::LINE_STRIP);
    // Add parameters to bundle
    if (bundle.bundleIndex() == 0) {
      bundle << X;
    } else if (bundle.bundleIndex() == 1) {
      bundle << Y;
    } else if (bundle.bundleIndex() == 2) {
      bundle << Rotation;
    }
  }

  void draw(Graphics &g) {
    g.pushMatrix();
    // Rotate and color according to bundle index to help differentiate
    g.color(0.4 + 0.2 * bundle.bundleIndex());
    g.translate(X, Y, -8.0);
    g.rotate(30 + 15 * bundle.bundleIndex() + Rotation, 1.0, 0.0, 0.0);
    g.draw(mesh);
    g.popMatrix();
  }

 private:
  Mesh mesh;
};

struct MyApp : public App {
  MyApp() {
    for (int i = 0; i < 3; i++) {
      // Create element
      auto *newThing =
          new Thing;  // This memory is not freed and it should be...
      things.push_back(newThing);
      // Register its parameter bundle with the ControlGUI
      gui << newThing->bundle;
    }
  }

  void onCreate() override {
    navControl().disable();
    gui.init();
  }

  void onDraw(Graphics &g) override {
    g.clear();
    for (Thing *thing : things) {
      thing->draw(g);
    }
    gui.draw(g);
  }

 private:
  std::vector<Thing *> things;
  ControlGUI gui;
};

int main() {
  MyApp app;
  app.start();

  return 0;
}
