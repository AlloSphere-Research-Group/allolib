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
  ControlGUI gui;

  PresetSequencer sequencer;
  SequenceRecorder recorder;

  void onCreate() override {
    addSphere(m, 0.2);
    nav().pullBack(4);
    navControl().disable();

    sequencer.setDirectory("presets");
    recorder.setDirectory("presets");
    // Register parameters with sequencer
    sequencer << X << Y;
    // Register parameters with recorder
    recorder << X << Y;

    // GUI
    gui.init();
    gui << X << Y;
    gui << sequencer << recorder;
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    g.translate(X, Y, 0.0);
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
};

// Function to write needed files for this example.
void writeExampleFile() {
  string sequence = R"(+0.0:/x:-0.6
+1.0:/x:0.3
+0.5:/x:0.6
+0.5:/x:-0.2
::
)";
  ofstream fseq("presets/paramseq.sequence");
  fseq << sequence;
  fseq.close();
}

int main() {
  writeExampleFile();
  MyApp().start();
  return 0;
}
