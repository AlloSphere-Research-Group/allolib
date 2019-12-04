#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al/ui/al_SequenceRecorder.hpp"

using namespace al;
using namespace std;

// This file shows how you can use the PresetSequencer class to store and
// recall individual parameter changes (without having them be part of a
// preset)

struct MyApp : App {
  Mesh m;

  Parameter X{"x", "", 0.0, "", -2, 2};
  Parameter Y{"y", "", 0.0, "", -2, 2};
  Parameter Z{"z", "", 0.0, "", -2, 2};
  ControlGUI gui;

  PresetHandler presetHandler;
  PresetSequencer sequencer;
  SequenceRecorder recorder;

  void onCreate() override {
    addSphere(m, 0.2);
    nav().pullBack(4);
    navControl().disable();

    sequencer.setDirectory("presets");
    // Register preset handler and parameters with sequencer
    sequencer << presetHandler << Z;
    // Register preset handler and parameters with recorder
    recorder << presetHandler << Z;

    // We have registered parameter Z with the sequencer, so we will register
    // the other two parameters with the preset handler. This setup will
    // store X and Y in preset files and changes to Z in the preset sequence
    // text file
    presetHandler << X << Y;

    gui.init();
    gui << X << Y << Z;
    gui << presetHandler;
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    g.translate(X, Y, Z);
    if (recorder.recording()) {
      g.color(1.0, 0.0, 0.0);
    } else if (sequencer.running()) {
      g.color(0.0, 1.0, 0.0);
    } else {
      g.color(0.0, 0.0, 1.0);
    }
    g.draw(m);
    gui.draw(g);
  }

  bool onKeyDown(const Keyboard &k) override {
    if (k.key() == ' ') {
      if (recorder.recording()) {
        recorder.stopRecord();
        std::cout << "End recording" << std::endl;
        auto steps = sequencer.loadSequence("seq");
        for (auto &step : steps) {
          std::cout << step.waitTime << " " << step.presetName << std::endl;
        }
      } else {
        recorder.startRecord("seq", true);
        std::cout << "Start recording" << std::endl;
      }
    } else if (k.key() == Keyboard::ENTER) {
      if (sequencer.running()) {
        sequencer.stopSequence();
        cout << "Sequencer stopped" << endl;
      } else {
        sequencer.playSequence("seq");
        cout << "Sequencer started" << endl;
      }
    }
    return true;
  }
};

int main() {
  MyApp().start();
  return 0;
}
