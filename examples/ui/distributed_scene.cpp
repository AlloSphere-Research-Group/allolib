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
#include "al/util/scene/al_DistributedScene.hpp"
#include "al/core/app/al_DistributedApp.hpp"
#include "al/core/graphics/al_Shapes.hpp"

using namespace gam;
using namespace al;

// This example shows how the DistributedScene class can be used
// to propagate changes in a DynamicScene across the network.
// Using DistributedApp together with DistributedScene, allows
// multiple instances of this same application binary to take
// on different roles. Try running two instances of this application
// on the same machine. The first will be the "primary" application
// controlling changes, and the second one will be a "replica"
// mirroring all the changes.

// See the examples for DynamicScene (dynamic_scene.cpp and
// avSequencer.cpp) for information on how it works.


// The Scene will contain "SimpleVoice" agents
class SimpleVoice : public PositionedVoice {
public:
    SimpleVoice()
    {
        mAmpEnv.levels(0,1,1,0);
        mAmpEnv.lengths(2, 0.5, 2);
        addTorus(mMesh);
        mMesh.primitive(Mesh::LINE_STRIP);

        // Register mFreq as the only parameter of the voice
        *this << mFreq;

        // Change the oscillator's frequency whenever the parameter value changes
        mFreq.registerChangeCallback([this](float value) {
            mOsc.freq(value);
        });
    }

    virtual void update(double dt) override {
        pose().vec().y = mAmpEnv.value();
        pose().vec().x = mFreq/440.0;
    }

    virtual void onProcess(AudioIOData& io) override {
        while(io()){
            io.out(0) += mOsc() * mAmpEnv() * 0.1;
        }
        if(mAmpEnv.done()) free();
    }

    virtual void onProcess(Graphics &g) override {
        g.color(mAmpEnv.value());
        g.draw(mMesh);
    }

    virtual void onTriggerOn() override {
        mAmpEnv.reset();
    }

    void updateFreq() {
        mFreq = mFreq * 0.992;
    }

protected:

    Parameter mAmp {"Amp"};
    Parameter mFreq {"Freq"};
    Parameter mDur {"Dur"};

    Sine<> mOsc;
    Env<3> mAmpEnv;

    Mesh mMesh;
};


// make an app that contains a SynthSequencer class
// use the render() functions from the SynthSequencer to produce audio and
// graphics in the corresponding callback
class MyApp : public DistributedApp<>
{
public:

    virtual void onCreate() override {

        if (isPrimary()) {
            title("Primary");

            // Trigger one voice manually
            auto *freeVoice = scene.getVoice<SimpleVoice>();
            std::vector<float> params{440.0f}; 
            freeVoice->setParamFields(params);
            freeVoice->pose().vec().z = -10.0;
            scene.triggerOn(freeVoice);
        } else {
            title("Replica");
        }

        parameterServer().print();
        scene.showWorldMarker(false);
        scene.registerSynthClass<SimpleVoice>();

        registerDynamicScene(scene);
    }

    virtual void onAnimate(double dt) override {
        if (isPrimary()) {
            // Only primary node updates frequency. The replicas get notified
            auto *voice = scene.getActiveVoices();
            while (voice) {
                static_cast<SimpleVoice *>(voice)->updateFreq();
                voice = voice->next;
            }
        }
        scene.update(dt);
    }

    virtual void onSound(AudioIOData &io) override {
        scene.render(io); // Render audio
    }

    virtual void onDraw(Graphics &g) override {
        g.clear();
        scene.render(g); // Render graphics
    }

//    virtual void onMessage(osc::Message &m) override {
//        scene.consumeMessage(m);
//    }

    virtual void onKeyDown(Keyboard const &k) override {
        if (k.key() == ' ') {
            auto *freeVoice = scene.getVoice<SimpleVoice>();
            std::vector<float> params{440.0f}; 
            freeVoice->setParamFields(params);
            freeVoice->pose().vec().z = -10.0;
            scene.triggerOn(freeVoice);
        }
    }

    DistributedScene scene{PolySynth::TIME_MASTER_CPU};
};

int main(){
    // Create app instance

    MyApp app;

    // Start everything
    app.initAudio(48000., 1024, 2,0);
    Domain::master().spu(app.audioIO().framesPerSecond());
    app.start();
}
