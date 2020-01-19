#include <fstream>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetHandler.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al/ui/al_SequenceServer.hpp"

using namespace al;
using namespace std;

struct MyApp : App {
  Mesh m;

  Parameter X{"x", "", 0.0, "", -2, 2};
  Parameter Y{"y", "", 0.0, "", -2, 2};

  PresetHandler presetHandler{"sequencerDir", true};
  PresetSequencer sequencer;
  // The sequencer server triggers sequences when it receives a valid sequence
  // name on OSC path /sequence
  // If you send a message using a command like:
  // oscsend 127.0.0.1 9012 /sequence s "seq"
  // You will trigger the sequence
  SequenceServer sequencerServer;  // Send OSC to 127.0.0.1:9011

  void onCreate() override {
    addSphere(m, 0.2);
    nav().pullBack(4);

    presetHandler << X << Y;       // Register parameters with preset handler
    sequencer << presetHandler;    // Register preset handler with sequencer
    sequencerServer << sequencer;  // Register sequencer with sequence server
    sequencerServer.print();

    sequencer.registerTimeChangeCallback(
        [](float time) { std::cout << time << std::endl; }, 0.5);
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    if (sequencer.running()) {
      g.translate(X.get(), Y.get(), 0);
      g.color(0.0, 1.0, 0.0);
    } else {
      g.color(0.0, 0.0, 1.0);
    }
    g.draw(m);
  }

  bool onMouseDown(const Mouse &m) override {
    if (sequencer.running()) {
      sequencer.stopSequence();
      cout << "Sequencer stopped" << endl;
    } else {
      sequencer.playSequence("seq");
      cout << "Sequencer started" << endl;
    }
    return true;
  }
};

void writeExamplePresets() {
  string sequence = R"(preset1:0.0:0.5
preset2:3.0:1.0
preset3:1.0:0.0
preset2:1.5:2.0
::
)";
  ofstream fseq("sequencerDir/seq.sequence");
  fseq << sequence;
  fseq.close();

  string preset1 = R"(::preset1
/x f 0.4
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

int main(int argc, char *argv[]) {
  writeExamplePresets();
  MyApp().start();
}
