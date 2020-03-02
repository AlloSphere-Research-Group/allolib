
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterServer.hpp"

using namespace al;

// This example shows registering parameters with an App's parameter server
// Pressing the 'a' and 'd' keys send OSC messages on localhost that will
// be picked up by this server.

struct MyApp : public App {
  Parameter X{"X", "", 0.0f, "", -1.0f, 1.0f};
  Parameter Y{"Y", "", 0.0f, "", -1.0f, 1.0f};

  ControlGUI gui;

  Mesh mesh;

  void onCreate() override {
    // Add Cone shape to mesh
    addCone(mesh);
    mesh.primitive(Mesh::LINE_STRIP);
    // Register parameters with GUI
    gui << X << Y;
    // Initialize control GUI
    gui.init();
    // Register parameters with App's internal parameter server
    parameterServer() << X << Y;
    parameterServer().print();
    // You can change the port and listening address of the parameter server at
    // runtime
    parameterServer().listen(9011, "localhost");
    parameterServer().print();

    navControl().disable();  // Disable Keyboard and mouse navigaion
  }

  void onDraw(Graphics& g) override {
    g.clear();
    g.pushMatrix();
    g.translate(X, Y, -8.0);
    g.draw(mesh);
    g.popMatrix();
    gui.draw(g);
  }

  bool onKeyDown(Keyboard const& k) override {
    // Use the 'd' and 'a' keys to change the X value
    // This sends an OSC message that will be receieved by the
    // parameter server
    if (k.key() == 'd') {
      osc::Send sender(parameterServer().serverPort());
      sender.send("/X", X + 0.1f);
    }

    if (k.key() == 'a') {
      osc::Send sender(parameterServer().serverPort());
      sender.send("/X", X - 0.1f);
    }
    return true;
  }
};

int main() {
  MyApp app;
  app.start();

  return 0;
}
