// OutputMaster example
// By Andrés Cabrera mantaraya36@gmail.com
// July 2018

#include <iostream>


#include "al/core.hpp"
#include "al/util/sound/al_OutputMaster.hpp"

using namespace al;

class MyApp : public App {
public:
    void onCreate() override {
        mOutputMaster.setMeterOn(true);
        mOutputMaster.setMeterUpdateFreq(10);
    }

    void onSound(AudioIOData &io) override {
        while(io()) {
            io.out(0) = io.in(0)* 0.1;
            io.out(1) = io.in(1)* 0.1;
        }
        mOutputMaster.onAudioCB(io);
    }

    void onDraw(Graphics &g) override {
        g.clear(0);
        float values[2];
        // Copies the current values to the array passed
        mOutputMaster.getCurrentValues(values);
        Mesh m;
        addQuad(m, 0.2, 0.2);

        g.color(1.0);
        g.pushMatrix();
        g.translate(-0.5, values[0]* 2, -2);
        g.draw(m);
        g.popMatrix();
        g.pushMatrix();
        g.translate(0.5, values[1]* 2, -2);
        g.draw(m);
        g.popMatrix();

    }

private:
    OutputMaster mOutputMaster {2,  44100};
};

int main(int argc, char *argv[])
{
    MyApp app;
    app.dimensions(640, 480);
    app.title("Stereo Audio Scene");
    app.fps(30);
    app.initAudio(44100, 256, 2, 2);
    app.start();
    return 0;
}

