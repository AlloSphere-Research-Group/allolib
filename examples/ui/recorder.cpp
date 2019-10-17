
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"

#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetHandler.hpp"
#include "al/ui/al_SequenceRecorder.hpp"

#include <fstream>

using namespace al;
using namespace std;

struct MyApp : App {
  Parameter X{"x", "", 0.0, "", -2, 2};
  Parameter Y{"y", "", 0.0, "", -2, 2};

  PresetHandler presetHandler{"data/sequencerDir", true};
  SequenceRecorder recorder;
  Mesh m;

  void onInit() override {
    presetHandler << X << Y;    // Register parameters with preset handler
    recorder << presetHandler;  // Register preset handler with sequencer
  }

  void onCreate() override {
    // Disable mouse nav to avoid naving while changing gui controls.
    navControl().useMouse(false);

    addSphere(m, 0.2);
    nav().pullBack(4);
  }

  void onDraw(Graphics& g) override {
    g.clear();
    g.translate(X.get(), Y.get(), 0);
    g.draw(m);
  }

  bool onKeyDown(const Keyboard& k) override {
    switch (k.key()) {
      case 'r':
        recorder.startRecord();
        break;
      case ' ':
        recorder.stopRecord();
        break;
      case '1':
        presetHandler.recallPreset("preset1");
        break;
      case '2':
        presetHandler.recallPreset("preset2");
        break;
      case '3':
        presetHandler.recallPreset("preset3");
        break;
    }
    return true;
  }
};

int main() {
  MyApp().start();
  return 0;
}
