#include "al/core.hpp"
#include "al/core/sound/al_Lbap.hpp"
#include "al/core/sound/al_Speaker.hpp"
#include "al/core/math/al_Random.hpp"
#include "al/util/al_AlloSphereSpeakerLayout.hpp"
#include "al/util/ui/al_Parameter.hpp"

#include <atomic>
#include <vector>

using namespace al;
using namespace std;

#define BLOCK_SIZE (512)

class MyApp : public App
{
    Mesh mPoly;

    SpeakerLayout speakerLayout;
    Lbap *panner {nullptr};
    float speedMult = 0.04f;
    double mElapsedTime = 0.0;

    ParameterVec3 srcpos {"srcPos", "", {0.0,0.0,0.0}};
    atomic<float> *mPeaks {nullptr};

public:
    MyApp()  {
    }

    ~MyApp() override {
        if (panner) { free(panner); }
        if (mPeaks) { free(mPeaks); }
    }

    void onInit() override {

        speakerLayout = AlloSphereSpeakerLayout();

        panner = new Lbap(speakerLayout);
        panner->compile();
        panner->print();
        panner->prepare(audioIO());

        //Determine number of output channels
        Speakers allSpeakers = speakerLayout.speakers();
        int highestChannel = 0;
        for(Speaker s:allSpeakers){
            if((int) s.deviceChannel > highestChannel){
                highestChannel = s.deviceChannel;
            }
        }

        audioIO().close();
        audioIO().channelsOut(highestChannel + 1);
        audioIO().open();

        mPeaks = new atomic<float>[speakerLayout.speakers().size()]; // Not being freed in this example
    }

    void onCreate() override {
        addDodecahedron(mPoly);
        nav().pos(0, 1, 25);
    }

    void onAnimate(double dt) override {
        // Move source position
        mElapsedTime += dt;
        float tta = mElapsedTime * speedMult * 2.0 * M_PI;

        float x = 5.0*cos(tta);
        float y = 3.0*sin(2.8 * tta);
        float z = 5.0*sin(tta);

        srcpos = Vec3d(x,y,z);
    }

    void onDraw(Graphics& g) override {
        g.clear(0);
        g.blending(true);
        g.blendModeTrans();
        Vec3d srcPosDraw = srcpos.get();

        //Draw the speakers
        Speakers sp = speakerLayout.speakers();
        for(int i = 0; i < (int) sp.size(); ++i){

            g.pushMatrix();
            float xyz[3];
            sp[i].posCart(xyz);
            g.translate(-xyz[1], xyz[2], -xyz[0]);
            float peak = mPeaks[i].load();
            g.scale(0.02 +  peak * 5);
            g.color(1);
            g.polygonLine();
            g.draw(mPoly);
            g.popMatrix();
        }

        // Draw line
        Mesh lineMesh;
        lineMesh.vertex(0.0,0.0, 0.0);
        lineMesh.vertex(srcPosDraw.x,0.0, srcPosDraw.z);
        lineMesh.vertex(srcPosDraw.x,srcPosDraw.y, srcPosDraw.z);
        lineMesh.index(0);
        lineMesh.index(1);
        lineMesh.index(1);
        lineMesh.index(2);
        lineMesh.index(2);
        lineMesh.index(0);
        lineMesh.primitive(Mesh::LINES);
        g.draw(lineMesh);

        //Draw the source
        g.pushMatrix();
        g.translate(srcPosDraw);
        g.scale(0.8);
        g.color(1,1, 1, 0.5);
        g.polygonFill();
        g.draw(mPoly);
        g.popMatrix();
    }

    virtual void onSound(AudioIOData & io) override {
        static unsigned int t = 0; // overall sample counter
        float srcBuffer[BLOCK_SIZE];

        // Render signal to be panned
        while (io()) {
            int i = io.frame();
            float env = (22050 - (t % 22050))/22050.0;
            srcBuffer[i] = 0.5 * rnd::uniform() * env;
            ++t;
        }
        // Pass signal to spatializer
        Pose pose;
        pose.pos(srcpos.get());
        panner->renderBuffer(io, pose, srcBuffer, BLOCK_SIZE);

        // Now compute RMS to display the signal level for each speaker
        Speakers &speakers = speakerLayout.speakers();
        for (int speaker = 0; speaker < (int) speakers.size(); speaker++) {
            float rms = 0;
            for (int i = 0; i < (int) io.framesPerBuffer(); i++) {
                int deviceChannel = speakers[speaker].deviceChannel;
                float sample = io.out(deviceChannel, i);
                rms += sample * sample;
            }
            rms = sqrt(rms/io.framesPerBuffer());
            mPeaks[speaker].store(rms);
        }
    }
};


int main (int argc, char * argv[])
{
    MyApp app;

    // Set up Audio
    AudioDevice::printAll();
    app.initAudio(44100, BLOCK_SIZE, 60, -1,
                  0);
    // Use this for sphere
//    app.initAudio(44100, BLOCK_SIZE, -1, -1,
//                  AudioDevice("ECHO X5").id());

    app.start();
    return 0;
}



