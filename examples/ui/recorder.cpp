
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"

#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_Preset.hpp"
#include "al/ui/al_SequenceRecorder.hpp"

#include <fstream>

using namespace al;
using namespace std;


struct MyApp : App
{
  Parameter X{"x", "", 0.0, "", -2, 2};
  Parameter Y{"y", "", 0.0, "", -2, 2};

  PresetHandler presetHandler{"data/sequencerDir", true};
  SequenceRecorder recorder;
  Mesh m;

  void onInit() {

    presetHandler << X << Y;   // Register parameters with preset handler
    recorder << presetHandler; // Register preset handler with sequencer

  }

  void onCreate() override{

    addSphere(m, 0.2);
    nav().pullBack(4);
  }

  void onDraw(Graphics& g) override {
    g.translate(X.get(), Y.get(), 0);
    g.draw(m);
  }

  void onKeyDown(const Keyboard& k) override {
    switch (k.key()) {
    case 'r': recorder.startRecord();
      break;
    case ' ': recorder.stopRecord();
      break;
    case '1': presetHandler.recallPreset("preset1");
      break;
    case '2': presetHandler.recallPreset("preset2");
      break;
    case '3': presetHandler.recallPreset("preset3");
      break;
    }
  }
};

int main(int argc, char* argv[])
{

  MyApp().start();
  return 0;
}
