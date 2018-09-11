#include "al/core.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/util/ui/al_Preset.hpp"

using namespace al;
using namespace std;

Parameter X("X", "Position", 0.0, "", -1.0, 1.0);
Parameter Y("Y", "Position", 0.0, "", -1.0, 1.0);
Parameter Size("Scale", "Size", 0.0, "", 0.1, 3.0);

// set the preset root directory here
PresetHandler presets("data/presets-example");

struct MyApp : App
{
  Light light;
  Mesh m;

  void onCreate() override {
    addCone(m);
    nav().pos(Vec3d(0, 0, 8));
    nav().setHome();
  }

  void onAnimate(double dt) override {
      static int counter = 0;
      counter++;
      // Update the positions every 15 frames
      if (counter == 30) {
          X = rnd::uniform(-1.0, 1.0);
          Y = rnd::uniform(-1.0, 1.0);
          Size = rnd::uniform(0.01, 0.6);
          counter = 0;
      }
  }

  void onDraw(Graphics &g) override {
  	g.clear(0);
    g.lighting(true);
    g.light(light);
    g.pushMatrix();
    g.translate(X.get(), Y.get(), 0);
    g.scale(Size.get());
    g.draw(m);
    g.popMatrix();
  }

  virtual void onKeyDown(const Keyboard &k) override {
    if (k.alt()) {
      switch (k.key()) {
      case '1':
        presets.storePreset("preset1");
        std::cout << "Preset 1 stored." << std::endl;
        break;
      case '2':
        presets.storePreset("preset2");
        std::cout << "Preset 2 stored." << std::endl;
        break;
      case '3':
        presets.storePreset("preset3");
        std::cout << "Preset 3 stored." << std::endl;
        break;
      }
    }
    else {
      switch (k.key()) {
      case '1':
        presets.recallPreset("preset1");
        std::cout << "Preset 1 loaded." << std::endl;
        break;
      case '2':
        presets.recallPreset("preset2");
        std::cout << "Preset 2 loaded." << std::endl;
        break;
      case '3':
        presets.recallPreset("preset3");
        std::cout << "Preset 3 loaded." << std::endl;
        break;
      }
    }
  }

};

int main(int argc, char *argv[]) {
  cout << "Press 1, 2 or 3 to recall preset, add alt key to store." << endl;
  presets << X << Y << Size; // Add parameters to preset handling
  presets.setSubDirectory("bank1");

  PresetServer presetServer("127.0.0.1", 9012);
  presetServer << presets; // Expose preset management through OSC
  // This address will be notified whenever the preset changes
  presetServer.addListener("127.0.0.1", 13560);

  MyApp app;
  app.dimensions(800, 600);
  app.title("Presets");
  app.fps(30);
  app.start();
}
