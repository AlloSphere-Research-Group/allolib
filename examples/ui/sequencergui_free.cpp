#include <fstream>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetHandler.hpp"
#include "al/ui/al_PresetSequencer.hpp"

using namespace al;
using namespace std;

// This example shows usage of the preset handler and preset sequencer in
// TIME_MASTER_FREE mode. This means that instead of running the sequencer
// and preset morphing in a separate CPU thread, the user is responsible
// of calling the step functions. This also means that the user must call
// setMorphStepTime() for the preset handler.

// Using the TIME_MASTER_FREE mode usually results in smoother graphics display
// But might incur in some jitter if you are also reading the parameters in
// other contexts (e.g. the audio context).

// See sequencergui.cpp example for details on PresetHandler and PresetSequencer

struct MyApp : App {
  Mesh m;

  Parameter X{"x", "", 0.0, "", -2, 2};
  Parameter Y{"y", "", 0.0, "", -2, 2};

  PresetHandler presetHandler{TimeMasterMode::TIME_MASTER_FREE, "sequencerDir",
                              true};
  PresetSequencer sequencer{TimeMasterMode::TIME_MASTER_FREE};

  ControlGUI gui;

  void onCreate() override {
    addSphere(m, 0.2);
    nav().pullBack(4);

    navControl().disable();

    presetHandler << X << Y;     // Register parameters with preset handler
    sequencer << presetHandler;  // Register preset handler with sequencer
    gui << sequencer;
    gui.init();

    // Currently the preset handler requires setting this manually.
    // In the future it will get picked up through the value passed
    // in stepMorphing()
    presetHandler.setMorphStepTime(1.0f / graphicsDomain()->fps());
  }

  void onAnimate(double dt) override {
    // As both the sequencer and the preset handler are in ASYNC mode, their
    // step functions must be called
    sequencer.stepSequencer(dt);
    presetHandler.stepMorphing(dt);
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
  return 0;
}
