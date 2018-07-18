// AudioScene example using the Stereo panner class
// By Andrés Cabrera mantaraya36@gmail.com
// March 7 2017 updated March 2018

#include <iostream>


#include "al/core.hpp"
#include "al/core/sound/al_StereoPanner.hpp"
#include "al/util/audio/al_OutputMaster.hpp"

#define BLOCK_SIZE 256

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
        values[0] = mOutputMaster.getCurrentChannelValue(0);
        values[1] = mOutputMaster.getCurrentChannelValue(1);
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
    app.initAudio(44100, BLOCK_SIZE, 2, 2);
    app.start();
    return 0;
}

