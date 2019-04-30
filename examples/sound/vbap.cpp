#include "al/core.hpp"
#include "al/core/sound/al_Vbap.hpp"
#include "al/core/sound/al_Speaker.hpp"
#include "al/util/al_Font.hpp"
#include "al/util/al_AlloSphereSpeakerLayout.hpp"

#include <atomic>
#include <vector>

using namespace al;
using namespace std;

#define BLOCK_SIZE (2048)


struct MyApp : public App
{
	Mesh mSpeakerMesh;
	vector<Mesh> mVec;

	vector<int>  sChannels;

    SpeakerLayout speakerLayout;
    Vbap *panner {nullptr};
    atomic<float> *mPeaks {nullptr};
    float speedMult = 0.03f;

    Vec3d srcpos {0.0,0.0,0.0};


public:
	MyApp()  {

	}

    virtual ~MyApp() {
        if (panner) free(panner);
        if (mPeaks) free(mPeaks);
    }

    void onInit() override {

        speakerLayout = AlloSphereSpeakerLayout();
        speakerLayout.addSpeaker(Speaker(12, 0, 90, 0, 5)); // Phantom speakers (top)
        speakerLayout.addSpeaker(Speaker(13, 0, -90, 2, 5)); // Phantom speakers (bottom)

        panner = new Vbap(speakerLayout, true);
        panner->makePhantomChannel(12, {0,1,2,3,4,5,6,7,8,9,10,11}); // assigned speakers top
        panner->makePhantomChannel(13, {0,1,2,3,4,5,6,7,8,9,10,11}); // assigned speakers bottom
        panner->compile();
        panner->print();

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

        panner->prepare(audioIO());

        mPeaks = new atomic<float>[speakerLayout.speakers().size()];

        vector<SpeakerTriple> sts = panner->triplets();

        //Setup the triangles
		for(int j = 0; j < sts.size();++j){
			SpeakerTriple &s = sts[j];
            Mesh mesh;
			mesh.primitive(Mesh::TRIANGLES);

			Color c(rnd::uniform(1.f,0.1f), rnd::uniform(), rnd::uniform(),0.4);
			for(int i = 0;i < 3;++i){
                mesh.vertex(-s.vec[i][1], s.vec[i][2], -s.vec[i][0]);
				mesh.color(c);
			}
            mesh.decompress();
            mesh.generateNormals();
			mVec.push_back(mesh);
		}

        addSphere(mSpeakerMesh, 1.0, 5, 5);

    }

	void onCreate() override {
		nav().pos(0, 1, 20);
	}

    void onAnimate( double dt) override {
        nav().faceToward(Vec3f(0,0,0)); // Always face origin
    }


	void onDraw(Graphics& g){
		g.clear(0);
		g.blending(true);
		g.blendModeAdd();

		//Draw the triangles
		g.meshColor();
        g.polygonFill();
		for(int i = 0; i < mVec.size(); ++i){
			g.draw(mVec[i]);
		}

		//Draw the source
		g.pushMatrix();
		g.translate(srcpos);
		g.scale(0.3);
        g.color(0.4,0.4, 0.4, 0.5);
		g.draw(mSpeakerMesh);
		g.popMatrix();

        // Draw line
        Mesh lineMesh;
        lineMesh.vertex(0.0,0.0, 0.0);
        lineMesh.vertex(srcpos.x,0.0, srcpos.z);
        lineMesh.vertex(srcpos);
        lineMesh.index(0);
        lineMesh.index(1);
        lineMesh.index(1);
        lineMesh.index(2);
        lineMesh.index(2);
        lineMesh.index(0);
        lineMesh.primitive(Mesh::LINES);
        g.color(1);
        g.draw(lineMesh);

		//Draw the speakers
		for(int i = 0; i < speakerLayout.numSpeakers(); ++i){
			g.pushMatrix();
			g.translate(speakerLayout.speakers()[i].vecGraphics());
			float peak = mPeaks[i].load();
			g.scale(0.02 + peak * 6);
            g.polygonLine();
			g.color(1);
			g.draw(mSpeakerMesh);
			g.popMatrix();
		}
	}

    void onSound(AudioIOData &io) override {
        static unsigned int t = 0;
        double sec;
        float srcBuffer[BLOCK_SIZE];

        while (io()) {
            int i = io.frame();
            float env = (22050 - (t % 22050))/22050.0;
            sec = (t / io.fps());
            // Signal is computed every sample
            srcBuffer[i] = 0.5 * rnd::uniform() * env;
            ++t;
        }
        // But the positions can be computed once per buffer
        float tta = sec*speedMult*M_2PI + M_2PI;
        float x = 6.0*cos(tta);
        float y = 6.0*sin(2.8 * tta);
        float z = 6.0*sin(tta);

        srcpos = Vec3d(x,y,z);

        panner->renderBuffer(io, srcpos, srcBuffer, BLOCK_SIZE);

        // Now compute RMS to display the signal level for each speaker
        Speakers &speakers = speakerLayout.speakers();
        for (int i = 0; i < speakers.size(); i++) {
            mPeaks[i].store(0);
        }
        for (int speaker = 0; speaker < speakers.size(); speaker++) {
            float rms = 0;
            for (int i = 0; i < io.framesPerBuffer(); i++) {
                int deviceChannel = speakers[speaker].deviceChannel;
                if(deviceChannel < io.channelsOut()) {
                    float sample = io.out(speakers[speaker].deviceChannel, i);
                    rms += sample * sample;
                }
            }
            rms = sqrt(rms/io.framesPerBuffer());
            mPeaks[speaker].store(rms);
        }
    }
};



int main (int argc, char * argv[]){

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



