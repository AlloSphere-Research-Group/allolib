/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:
    Description:
*/

#include <cstdio>               // for printing to stdout
#define GAMMA_H_INC_ALL         // define this to include all header files
#define GAMMA_H_NO_IO           // define this to avoid bringing AudioIO from Gamma

#include "Gamma/Gamma.h"

#include "al/core/io/al_AudioIO.hpp"
#include "al/util/scene/al_SynthSequencer.hpp"
#include "al/core/app/al_App.hpp"
#include "al/core/graphics/al_Shapes.hpp"
Y
using namespace gam;
using namespace al;

// Create a sub class of SynthVoice to determine what each voice should do
// in the onProcess() audio and video callbacks.
// Add functions to set voice parameters (per instance parameters)
// Don't forget to define an onTriggerOn() function to reset envelopes or
// values for each triggering
class SineEnv : public SynthVoice {
public:

    SineEnv()
    {
        set (6.5, 60, 0.3, 0.1, 2);
        mAmpEnv.curve(0); // make segments lines
        mAmpEnv.levels(0,1,1,0);

        addSphere(mMesh, 0.2, 30, 30);
    }

    // Note parameters
    SineEnv& freq(float v){ mOsc.freq(v); return *this; }
    SineEnv& amp(float v){ mAmp=v; return *this; }
    SineEnv& attack(float v){
        mAmpEnv.lengths()[0] = v;
        return *this;
    }
    SineEnv& decay(float v){
        mAmpEnv.lengths()[2] = v;
        return *this;
    }

    SineEnv& dur(float v){ mDur=v; return *this; }

    SineEnv& pan(float v){ mPan.pos(v); return *this; }

    SineEnv& set(
        float a, float b, float c, float d, float e, float f=0
    ){
        return dur(a).freq(b).amp(c).attack(d).decay(e).pan(f);
    }

    //
    virtual void onProcess(AudioIOData& io) override {
        while(io()){
            float s1 = mOsc() * mAmpEnv() * mAmp;
            float s2;
            mEnvFollow(s1);
            mPan(s1, s1,s2);
            io.out(0) += s1;
            io.out(1) += s2;
        }
        if(mAmpEnv.done() && (mEnvFollow.value() < 0.001f)) free();
    }

    virtual void onProcess(Graphics &g) override {
        g.pushMatrix();
        g.blending(true);
        g.blendModeTrans();
        g.translate(mOsc.freq()/500 - 3,  pow(mAmp, 0.3), -8);
        g.scale(1- mDur, mDur, 1);
        g.color(1, mOsc.freq()/1000, mEnvFollow.value());
        g.draw(mMesh);
        g.popMatrix();
    }

    virtual void onTriggerOn() override {
        std::cout << "------- " << mDur << std::endl;
        mAmpEnv.totalLength(mDur, 1);
        if (mAmpEnv.lengths()[1] < 0.0f) {
            mAmpEnv.lengths()[1] = 0.0f;
        }
        mAmpEnv.reset();
    }

protected:

    float mAmp;
    float mDur;
    Pan<> mPan;
    Sine<> mOsc;
    Env<3> mAmpEnv;
    EnvFollow<> mEnvFollow;

    Mesh mMesh;
};


// make an app that contains a SynthSequencer class
// use the render() functions from the SynthSequencer to produce audio and
// graphics in the corresponding callback
class MyApp : public App
{
public:
    virtual void onSound(AudioIOData &io) override {
        s.render(io); // Render audio
    }

    virtual void onDraw(Graphics &g) override {
//        s.print(); // Prints information on active and free voices
        g.clear();
        s.render(g); // Render graphics
    }

    SynthSequencer s;
};

// Function to quickly create a set of notes
void makeLine(SynthSequencer &s, float startTime, int number,
              float duration, float startFreq, float endFreq, float amp) {

    for (int i = 0; i < number; i++) {
        s.add<SineEnv>(startTime + i*duration)
                .freq(startFreq + ((i/float(number)) * (endFreq - startFreq)))
                .attack(duration/2)
                .decay(duration/2)
                .dur(duration)
                .amp(amp);
    }

}


int main(){

    // Create app instance
    MyApp app;

    // Sequence events using the function defined above
    makeLine(app.s, 0, 10, 0.6, 440, 660, 0.05);
    makeLine(app.s, 3, 10, 0.4, 220, 660, 0.07);
    makeLine(app.s, 6, 10, 0.3, 220, 440, 0.08);
    makeLine(app.s, 10, 30, 0.2, 220, 440, 0.1);
    makeLine(app.s, 13, 80, 0.1, 220, 880, 0.12);
    makeLine(app.s, 18, 180, 0.05, 220, 440, 0.19);
    makeLine(app.s, 20, 10, 0.1, 880, 1600, 0.2);
    makeLine(app.s, 20, 10, 0.2, 880, 1600, 0.3);
    makeLine(app.s, 21, 10, 0.3, 440, 1600, 0.4);
    makeLine(app.s, 22, 20, 0.1, 220, 3000, 0.2);
    makeLine(app.s, 25, 30, 0.4, 3000, 1500, 0.19);
    makeLine(app.s, 27, 30, 0.5, 4000, 1000, 0.18);
    makeLine(app.s, 30, 50, 0.6, 3000, 500, 0.17);
    makeLine(app.s, 31, 10, 1.0, 880, 440, 0.1);

    // Start everything
    app.initAudio(44100., 256, 2, 0);
    Domain::master().spu(app.audioIO().framesPerSecond());
    app.start();
}
