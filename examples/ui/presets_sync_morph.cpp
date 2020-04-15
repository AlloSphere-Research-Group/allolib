

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_HtmlInterfaceServer.hpp"
#include "al/ui/al_ParameterMIDI.hpp"

using namespace al;

struct MyApp : public App {
  Parameter Size{"Size", "", 0.3, "", 0.1, 2.0};
  ParameterBool RunPresets{"Run Presets", "", 1.0};

  // Set time master mode to Async
  PresetHandler presets{TimeMasterMode::TIME_MASTER_FREE, "presetsGUI"};

  ControlGUI gui;
  Mesh mMeshCone;

  void onInit() override {
    // Add parameters to preset handling
    presets << Size;

    // Add to GUI
    gui << Size << RunPresets;

    // Adding a PresetHandler to a ControlGUI creates a multi-button interface
    // to control the presets.
    gui << presets;

    addCone(mMeshCone);
  }

  void onCreate() override {
    nav().pos(Vec3d(0, 0, 8));
    // Disable mouse nav to avoid naving while changing gui controls.
    navControl().useMouse(false);

    // Set morph step size. Since we will be stepping the morphing function
    // from the draw callback, we can compute step size from fps.
    presets.setMorphStepTime(1.0 / graphicsDomain()->fps());
    gui.init();
  }

  void onAnimate(double dt) override {
    // If you uncheck Run Presets, selecting presets will have no effect
    if (RunPresets.get() == 1.0) {
      // Set parameter values to next step in morph (if morphing)
      presets.stepMorphing();
    }
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    g.scale(Size.get());
    g.draw(mMeshCone);
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
