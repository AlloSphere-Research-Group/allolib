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
#include "al/util/ui/al_SynthSequencer.hpp"
#include "al/core/app/al_App.hpp"
#include "al/core/graphics/al_Shapes.hpp"

using namespace gam;
using namespace al;


Domain graphicsDomain; // This determines the clock for graphics processors

// Create a sub class of SynthVoice to determine what each voice should do
// in the onProcess() audio and video callbacks.
// Add functions to set voice parameters (per instance parameters)
// Don't forget to define an onTriggerOn() function to reset envelopes or
// values for each triggering
class SineEnv : public SynthVoice {
public:

    SineEnv()
    {
        set (1.5, 60, 0.3, 1, 2);
        mAmpEnv.curve(0); // make segments lines
        mAmpEnv.levels(0,1,1,0);
        mAmpEnv.sustainPoint(2);

        mSpatialEnv.domain(graphicsDomain); // This envelope runs in the graphics domain
        mSpatialEnv.levels(0, 3, 2.5, 0);
        mSpatialEnv.sustainPoint(2);
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

    virtual void onProcess(Graphics &g) {
        float spatialEnv = mSpatialEnv();
        g.pushMatrix();
        g.blendOn();
        g.translate(mOsc.freq()/500 - 3,  pow(mAmp, 0.3) + spatialEnv - 2, -8);
        g.scale(1- mDur, mDur, 1);
        g.color(mSpatialEnv(), mOsc.freq()/1000, mEnvFollow.value());
        g.draw(mMesh);
        g.popMatrix();
    }

    virtual void onTriggerOn() override {
        mAmpEnv.totalLength(mDur, 1);
        mAmpEnv.reset();
        mSpatialEnv.totalLength(mDur);
        mSpatialEnv.reset();
    }

    virtual void onTriggerOff() override {
        mAmpEnv.release();
        mSpatialEnv.release();
    }

protected:

    float mAmp;
    float mDur;
    Pan<> mPan;
    Sine<> mOsc;
    Env<3> mAmpEnv;
    EnvFollow<> mEnvFollow;

    Env<3> mSpatialEnv;
    Mesh mMesh;
};


// We will use PolySynth to handle voice triggering and allocation
class MyApp : public App
{
public:
    virtual void onSound(AudioIOData &io) override {
        pSynth.render(io); // Render audio
    }

    virtual void onDraw(Graphics &g) override {
        g.clear();
        pSynth.render(g);
    }

    virtual void onKeyDown(Keyboard const& k) override {
        int midiNote = asciiToMIDI(k.key());
        if (midiNote > 0) {
            float frequency = ::pow(2., (midiNote - 69.)/12.) * 440.;
            SineEnv *voice = pSynth.getVoice<SineEnv>();
            voice->freq(frequency);
            pSynth.triggerOn(voice, 0, midiNote);
        }
    }
    virtual void onKeyUp(Keyboard const& k) override {
        int midiNote = asciiToMIDI(k.key());
        if (midiNote > 0) {
            pSynth.triggerOff(midiNote);
        }
    }


    PolySynth pSynth;
};


int main(){

    // Create app instance
    MyApp app;

    // Pre-allocate voice to avoid real-time allocation
    app.pSynth.allocatePolyphony<SineEnv>(16);

    app.navControl().active(false); // Disable navigation via keyboard, since we will be using keyboard for note triggering

    // Start everything
    app.initAudio(44100., 256, 2, 0);

    // Set up processing domains
    Domain::master().spu(app.audioIO().framesPerSecond());
    graphicsDomain.spu(app.fpsWanted());

    app.start();
}
