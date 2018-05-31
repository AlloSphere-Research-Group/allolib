#include "al/core.hpp"
#include "al/core/sound/al_Lbap.hpp"
#include "al/core/sound/al_Speaker.hpp"
#include "al/util/al_AlloSphereSpeakerLayout.hpp"
#include "al/core/math/al_Random.hpp"

#include <atomic>
#include <vector>

using namespace al;
using namespace std;

#define BLOCK_SIZE (512)

static SpeakerLayout speakerLayout;
static Lbap *panner;
static float speedMult = 0.03f;

static float srcElev = 0.0f;

static Vec3d srcpos(0.0,0.0,0.0);
static atomic<float> *mPeaks;

////Currently bypasses al_AudioScene

static void audioCB(AudioIOData& io){
	// Render scene
	static unsigned int t = 0;
	double sec;
	float srcBuffer[BLOCK_SIZE];

	while (io()) {
		int i = io.frame();
		float env = (22050 - (t % 22050))/22050.0;
		
		// Signal is computed every sample
		srcBuffer[i] = 0.5 * rnd::uniform() * env;
		++t;
	}
	sec = (t / io.fps());
	// But the positions can be computed once per buffer
	float tta = sec*speedMult*M_2PI + M_2PI;
	
	float x = 5.0*cos(tta);
	float y = 2.0*sin(2.8 * tta);
	float z = 5.0*sin(tta);

	srcpos = Vec3d(x,y,srcElev + z);

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

class MyApp : public App
{
	Mesh mPoly;

	vector<Mesh> mVec;

	vector<Vec3d> sCoords;
	vector<int>  sChannels;

	Light mLight;


public:
	MyApp()  {
	}

	void onCreate() {
        addDodecahedron(mPoly);
		mLight.pos(0,6,0);

		nav().pos(0, 2, 45);
	}


	void onDraw(Graphics& g){
		g.clear(0);
		g.blending(true);
		g.blendModeAdd();

		//Draw the source
		g.pushMatrix();
		g.translate(srcpos);
		g.scale(0.4);
		g.color(1,1, 1, 0.5);
        g.polygonFill();
		g.draw(mPoly);
		g.popMatrix();

		//Draw the speakers

        Speakers sp = speakerLayout.speakers();
		for(int i = 0; i < sp.size(); ++i){

			g.pushMatrix();

            float xyz[3];
            sp[i].posCart(xyz);
			g.translate(-xyz[1], xyz[2], -xyz[0]);
			float peak = mPeaks[i].load();
			g.scale(0.02 + 0.06 * peak * 30);
			g.color(1);
            g.polygonLine();
			g.draw(mPoly);
			g.popMatrix();
		}

		// Draw line
		Mesh lineMesh;
		lineMesh.vertex(0.0,0.0, 0.0);
		lineMesh.vertex(srcpos.x,0.0, srcpos.z);
		lineMesh.vertex(srcpos.x,srcpos.y, srcpos.z);
		lineMesh.index(0);
		lineMesh.index(1);
		lineMesh.index(1);
		lineMesh.index(2);
		lineMesh.index(2);
		lineMesh.index(0);
		lineMesh.primitive(Mesh::LINES);
		g.draw(lineMesh);
	}
};



int main (int argc, char * argv[]){

	speakerLayout = AlloSphereSpeakerLayout();

	panner = new Lbap(speakerLayout);
    panner->compile();
	panner->print();

	//Determine number of output channels
	Speakers allSpeakers = speakerLayout.speakers();
	int highestChannel = 0;
	for(Speaker s:allSpeakers){
		if(s.deviceChannel > highestChannel){
			highestChannel = s.deviceChannel;
		}
	}

	mPeaks = new atomic<float>[speakerLayout.speakers().size()];

	int outputChannels = highestChannel + 1;

    AudioDevice::printAll();
    AudioDevice dev("ECHO X5");
	AudioIO audioIO;
    audioIO.init(audioCB, nullptr, BLOCK_SIZE, 44100, outputChannels, 0);
	audioIO.device(dev);


    panner->prepare(audioIO);
    audioIO.open();
	audioIO.start();

	MyApp().start();
	getchar();

	audioIO.stop();
	delete mPeaks;

	return 0;
}



