
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"

#include "al/math/al_Random.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetHandler.hpp"
#include "al/ui/al_PresetServer.hpp"

using namespace al;
using namespace std;

struct MyApp : App {
  Parameter X{"X", "Position", 0.0, "", -1.0f, 1.0f};
  Parameter Y{"Y", "Position", 0.0, "", -1.0f, 1.0f};
  Parameter Size{"Scale", "Size", 0.0, "", 0.1f, 3.0f};

  // set the preset root directory here
  PresetHandler presets{"data/presets-example"};
  PresetServer presetServer{"127.0.0.1", 9012};

  Light light;
  Mesh m;

  void onCreate() override {
    cout << "Press 1, 2 or 3 to recall preset, add alt key to store." << endl;
    presets << X << Y << Size;  // Add parameters to preset handling
    presets.setSubDirectory("bank1");

    presetServer << presets;  // Expose preset management through OSC
    // This address will be notified whenever the preset changes
    presetServer.addListener("127.0.0.1", 13560);

    addCone(m);
    nav().pos(Vec3d(0, 0, 8));
    nav().setHome();
  }

  void onAnimate(double /*dt*/) override {
    static int counter = 0;
    counter++;
    // Update the positions every 15 frames
    if (counter == 30) {
      X = rnd::uniform(-1.0f, 1.0f);
      Y = rnd::uniform(-1.0f, 1.0f);
      Size = rnd::uniform(0.01f, 0.6f);
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

  bool onKeyDown(const Keyboard &k) override {
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
    } else {
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
    return true;
  }
};

int main() {
  MyApp app;
  app.title("Presets");
  app.start();
  return 0;
}
