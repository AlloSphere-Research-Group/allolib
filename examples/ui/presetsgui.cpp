

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_HtmlInterfaceServer.hpp"
#include "al/ui/al_ParameterMIDI.hpp"

using namespace al;

struct MyApp : public App {
  ParameterInt Number{"Number", "", 1, "", 0, 16};
  ParameterMenu Shape{"Shape"};
  Parameter Size{"Size", "", 0.3, "", 0.1, 2.0};
  Parameter Red{"Red", "Color", 0.5, "", 0.0, 1.0};
  Parameter Green{"Green", "Color", 1.0, "", 0.0, 1.0};
  Parameter Blue{"Blue", "Color", 0.5, "", 0.0, 1.0};

  PresetHandler presets{"presetsGUI"};

  ControlGUI gui;

  Light light;
  Mesh mMeshCone;
  Mesh mMeshCube;

  void onInit() override {
    // Add parameters to preset handling
    presets << Number << Size << Red << Green << Blue << Shape;
    presets.verbose(true);

    Shape.setElements({"Cone", "Cube"});

    // Now make control GUI
    // You can add Parameter objects using the streaming operator.
    // They will all be laid out vertically
    gui << Number << Size;
    gui << Red << Green << Blue << Shape;

    // Expose parameters to network (You can send OSC message to them
    parameterServer() << Number << Size << Red << Green << Blue << Shape;

    // Print server configuration
    parameterServer().print();

    // Adding a PresetHandler to a ControlGUI creates a multi-button interface
    // to control the presets.
    gui << presets;

    addCone(mMeshCone);
    mMeshCone.generateNormals();
    addCube(mMeshCube);
    mMeshCube.generateNormals();
  }

  virtual void onCreate() override {
    nav().pos(Vec3d(0, 0, 8));
    // Disable mouse nav to avoid naving while changing gui controls.
    navControl().useMouse(false);

    gui.init();
  }

  virtual void onDraw(Graphics &g) override {
    g.clear(0);
    // light();
    for (int i = 0; i < Number.get(); ++i) {
      g.pushMatrix();
      g.translate((i % 4) - 2, (i / 4) - 2, -5);
      g.scale(Size.get());
      g.color(Red.get(), Green.get(), Blue.get());
      if (Shape.get() == 0) {
        g.draw(mMeshCone);
      } else {
        g.draw(mMeshCube);
      }
      g.popMatrix();
    }
    gui.draw(g);
  }
};

int main(int argc, char *argv[]) {
  MyApp app;
  app.dimensions(800, 600);
  app.title("Presets GUI");
  app.fps(30);
  app.start();
  return 0;
}
