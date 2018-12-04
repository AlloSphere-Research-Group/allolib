/*    Gamma - Generic processing library
    See COPYRIGHT file for authors and license information

    Example:
    Description:
*/

#include <cstdio>               // for printing to stdout
#define GAMMA_H_INC_ALL         // define this to include all header files
#define GAMMA_H_NO_IO           // define this to avoid bringing AudioIO from Gamma

#include "Gamma/Gamma.h"

//#include "al/core/io/al_AudioIO.hpp"
#include "al/util/scene/al_DynamicScene.hpp"
#include "al/core/app/al_App.hpp"
#include "al/core/graphics/al_Shapes.hpp"

using namespace gam;
using namespace al;

// A DynamicScene manages the insertion and removal of
// PositionedVoice nodes in its rendering graph.
// A DynamicScene has three rendering contexts:
// The update() or simulation context, where state and
// internal changes should be computed, and the onProcess()
// contexts for audio and graphics.

// The DynamicScene will contain "SimpleVoice" agents that
// inherit from PositionedVoice
class SimpleVoice : public PositionedVoice {
public:
    SimpleVoice()
    {
        mAmpEnv.levels(0,1,1,0);
        mAmpEnv.lengths(3.0, 1.0, 3.0);
        mAmpEnv.sustainDisable();
        addTorus(mMesh);
        mMesh.primitive(Mesh::LINE_STRIP);

        // Register parameters using the stream operator
        // parameters registered this way can be set through
        // the setParamFields(), and will also allow their
        // values to be stored when using SynthSequencer
        *this << mFreq;

        // The Freq parameter will drive changes in the
        // internal oscillator
        mFreq.registerChangeCallback([this](float value) {
            mOsc.freq(value);
        });
    }

    // The update function will change the position of the
    virtual void update(double dt) override {

        mFreq = mFreq * 0.995;
        pose().vec().y = mAmpEnv.value()*3;
        pose().vec().x = mFreq/440.0;
    }

    virtual void onProcess(AudioIOData& io) override {
        while(io()){
            io.out(0) += mOsc() * mAmpEnv() * 0.05;
        }
        if(mAmpEnv.value() < 0.001) free();
    }

    virtual void onProcess(Graphics &g) override {
        HSV c;
        c.h = mAmpEnv.value();
        g.color(Color(c));
        g.draw(mMesh);
    }

    virtual void onTriggerOn() override {

        pose().vec() = {mFreq/440.0 , 0.0, -10.0};
        mAmpEnv.reset();
    }

protected:
    Parameter mFreq {"Freq"};

    Sine<> mOsc;
    Env<3> mAmpEnv;

    Mesh mMesh;
};


// make an app that contains a SynthSequencer class
// use the render() functions from the SynthSequencer to produce audio and
// graphics in the corresponding callback
class MyApp : public App
{
public:

    virtual void onCreate() override {

        scene.showWorldMarker(false);
        scene.registerSynthClass<SimpleVoice>();
        // Preallocate 300 voices
        scene.allocatePolyphony("SimpleVoice", 300);
        scene.prepare(audioIO());
    }

    virtual void onAnimate(double dt) override {
        static double timeAccum = 0.1;
        timeAccum += dt;
        if (timeAccum > 0.1) {
        // Trigger one new voice every 0.05 seconds
            // First get a free voice of type SimpleVoice
            auto *freeVoice = scene.getVoice<SimpleVoice>();
            // Then set its parameters (this voice only has one parameter Freq)
            freeVoice->setParamFields(std::vector<float>{880.0f});
            // Set a position for it
            // Trigger it (this inserts it into the chain)
            scene.triggerOn(freeVoice);
            timeAccum -= 0.1;
        }

        scene.update(dt); // Update all nodes in the scene
    }

    virtual void onSound(AudioIOData &io) override {
        scene.render(io); // Render audio
    }

    virtual void onDraw(Graphics &g) override {
//        s.print(); // Prints information on active and free voices
        g.clear();
        // Render scene on the left
        g.pushMatrix();
        scene.render(g); // Render graphics
        g.popMatrix();
    }

    // The number passed to the construtor indicates how many threads are used to compute the scene
    DynamicScene scene {4};
};

int main(){
    // Create app instance
    MyApp app;

    // Start everything
    app.initAudio(44100., 512, 2,0);
    Domain::master().spu(app.audioIO().framesPerSecond());
    app.start();
}
