#include "al/core.hpp"

#include "al/util/ui/al_PresetSequencer.hpp"
#include "al/util/ui/al_SequenceRecorder.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/util/ui/al_ControlGUI.hpp"

using namespace al;
using namespace std;


struct MyApp : App
{
	Mesh m;

    Parameter X{"x", "", 0.0, "", -2, 2};
    Parameter Y{"y", "", 0.0, "", -2, 2};
    ControlGUI gui;

    PresetHandler presetHandler{"sequencerDir", true};
    PresetSequencer sequencer;
    SequenceRecorder recorder;

	void onCreate() override {
		addSphere(m, 0.2);
		nav().pullBack(4);

        sequencer << X << Y; // Register parameters with sequencer
        recorder << X << Y; // Register parameters with recorder

        gui.init();
        gui << X << Y;
	}

	void onDraw(Graphics &g) override {
		g.clear(0);
        g.translate(X.get(), Y.get(), 0);
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

