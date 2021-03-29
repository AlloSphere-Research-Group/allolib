
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterBundle.hpp"

using namespace al;

// This example shows how ParameterServer interacts with
// ParameterBundle. See the parameterbundle examples to understand
// ParameterBundle.

class Thing {
 public:
  Parameter X{"X", "", 0.0f, "", -1.0f, 1.0f};
  Parameter Y{"Y", "", 0.0f, "", -1.0f, 1.0f};
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
    g.color(0.4f + 0.2f * bundle.bundleIndex());
    g.translate(X, Y, -8.0);
    g.rotate(30 + 15 * bundle.bundleIndex(), 1.0, 0.0, 0.0);
    g.draw(mesh);
    g.popMatrix();
  }

 private:
  Mesh mesh;
};

class MyApp : public App {
 public:
  void onCreate() override {
    for (int i = 0; i < 3; i++) {
      // Create element
      auto *newThing =
          new Thing;  // This memory is not freed and it should be...
      things.push_back(newThing);
      // Register its parameter bundle with the ControlGUI
      gui << newThing->bundle;
      parameterServer() << newThing->bundle;
    }
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

  bool onKeyDown(Keyboard const &k) override {
    // Use the 'd' and 'a' keys to change the X value for first "thing"
    // This sends an OSC message that will be receieved by the
    // parameter server
    if (k.key() == 'd') {
      osc::Send sender(parameterServer().serverPort());
      sender.send("/thing/0/X", things[0]->X + 0.1f);
    }

    if (k.key() == 'a') {
      osc::Send sender(parameterServer().serverPort());
      sender.send("/thing/0/X", things[0]->X - 0.1f);
    }

    // Use the 'f' and 'h' keys to change the X value for first "thing"
    if (k.key() == 'h') {
      osc::Send sender(parameterServer().serverPort());
      sender.send("/thing/1/X", things[1]->X + 0.1f);
    }

    if (k.key() == 'f') {
      osc::Send sender(parameterServer().serverPort());
      sender.send("/thing/1/X", things[1]->X - 0.1f);
    }

    // Use the 'j' and 'l' keys to set the X value for current "thing"
    // The current "thing" for OSC is not the same as the one for the GUI.
    // Use keys 1,2,3 to select which thing to address
    if (k.key() == 'j') {
      osc::Send sender(parameterServer().serverPort());
      sender.send("/X", -0.5f);
    }
    if (k.key() == 'l') {
      osc::Send sender(parameterServer().serverPort());
      sender.send("/X", 0.5f);
    }
    if (k.key() == '1') {
      osc::Send sender(parameterServer().serverPort());
      sender.send("/thing/_current", 0);
    }
    if (k.key() == '2') {
      osc::Send sender(parameterServer().serverPort());
      sender.send("/thing/_current", 1);
    }
    if (k.key() == '3') {
      osc::Send sender(parameterServer().serverPort());
      sender.send("/thing/_current", 2);
    }
    return true;
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
