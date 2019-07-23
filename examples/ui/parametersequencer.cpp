#include "al/app/al_App.hpp"

#include "al/util/ui/al_PresetSequencer.hpp"
#include "al/util/ui/al_SequenceRecorder.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ControlGUI.hpp"

using namespace al;
using namespace std;


struct MyApp : App
{
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

        sequencer << presetHandler<< Z; // Register preset handler and parameters with sequencer
        recorder << presetHandler << Z; // Register preset handler and parameters with recorder
        presetHandler << X << Y;

        gui.init();
        gui << X << Y << Z;
        gui << presetHandler;
	}

	void onDraw(Graphics &g) override {
		g.clear(0);
        g.translate(X, Y, Z);
        if (recorder.recording()) {
			g.color(1.0,0.0,0.0);
		} else if (sequencer.running()) {
			g.color(0.0,1.0,0.0);
		} else {
			g.color(0.0,0.0,1.0);
		}
		g.draw(m);
        gui.draw(g);
	}

	void onMouseDown(const Mouse &m) override {
	}

    void onKeyDown(const Keyboard &k) override {
        if (k.key() == ' ') {
            if (recorder.recording()) {
                recorder.stopRecord();
                std::cout << "End recording" << std::endl;
                auto steps = sequencer.loadSequence("seq");
                while (!steps.empty()) {
                    std::cout << steps.front().waitTime << " " << steps.front().presetName << std::endl;
                    steps.pop();
                }
            } else {
                recorder.startRecord("seq", true);
                std::cout << "Start recording" <<std::endl;

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
    }
};

int main(int argc, char* argv[])
{
	MyApp().start();
}

