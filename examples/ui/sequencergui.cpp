#include <fstream>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetHandler.hpp"
#include "al/ui/al_PresetSequencer.hpp"

using namespace al;
using namespace std;

struct MyApp : App {
  Mesh m;

  Parameter X{"x", "", 0.0, "", -2, 2};
  Parameter Y{"y", "", 0.0, "", -2, 2};

  PresetHandler presetHandler{"sequencerDir", false};
  PresetSequencer sequencer;

  ControlGUI gui;

  void onCreate() override {
    addSphere(m, 0.2);
    nav().pullBack(4);

    navControl().disable();

    presetHandler << X << Y;     // Register parameters with preset handler
    sequencer << presetHandler;  // Register preset handler with sequencer
    gui << X << Y << sequencer;
    gui.init();

    // Sequencer timing will run in a separate thread by default.
    // You can set the sequencer "granularity", i.e. the minimum time
    // between sequencer steps.
    // A smaller number means less jitter in the sequencer at the expense
    // of greater CPU usage. The default value is 0.05 (50 ms). Using 0.01
    // Results is smoother visual movement.
    sequencer.setSequencerStepTime(0.01);
    // The begin callback is called whenever the sequence starts
    sequencer.registerBeginCallback([&](PresetSequencer *) {
      std::cout << "**** Started Sequence" << std::endl;
    });
    // The end callback is called when the sequence ends or is stopped
    // The 'finished' argument is true if the sequence finished by
    // itself. It will be false if the sequence was stopped by the user.
    sequencer.registerEndCallback([&](bool finished, PresetSequencer *) {
      if (finished) {
        std::cout << "**** Sequence FINSIHED ***" << std::endl;
      } else {
        std::cout << "**** Sequence Stopped" << std::endl;
      }
    });
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    if (sequencer.running()) {
      g.color(0.0, 1.0, 0.0);
    } else {
      g.color(0.0, 0.0, 1.0);
    }
    g.translate(X.get(), Y.get(), 0);
    g.draw(m);
    gui.draw(g);
  }
};

// Function to write needed files for this example.
void writeExamplePresets() {
  string sequence = R"(preset1:0.0:0.5
preset2:3.0:1.0
preset3:1.0:0.0
preset1:1.5:2.0
::
)";
  ofstream fseq("sequencerDir/seq.sequence");
  fseq << sequence;
  fseq.close();

  string preset1 = R"(::preset1
/x f -0.4
/y f 0.2
::
)";
  ofstream f1("sequencerDir/preset1.preset");
  f1 << preset1;
  f1.close();

  string preset2 = R"(::preset2
/x f 0.6
/y f -0.9
::
)";
  ofstream f2("sequencerDir/preset2.preset");
  f2 << preset2;
  f2.close();

  string preset3 = R"(::preset3
/x f -0.1
/y f 1.0
::
)";
  ofstream f3("sequencerDir/preset3.preset");
  f3 << preset3;
  f3.close();
}

int main() {
  writeExamplePresets();
  MyApp().start();
}
