#include "al/core.hpp"
#include "al/core/sound/al_Lbap.hpp"
#include "al/core/sound/al_Speaker.hpp"
#include "al/util/al_AlloSphereSpeakerLayout.hpp"

#include <atomic>
#include <vector>

using namespace al;
using namespace std;

#define BLOCK_SIZE (2048)

static SpeakerLayout speakerLayout;
static Lbap *panner;
static float speedMult = 0.01f;

static float srcElev = 1.6f;

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
		sec = (t / io.fps());
		// Signal is computed every sample
		srcBuffer[i] = 0.5 * sin(sec*220*M_2PI) * env;;
		++t;
	}
	// But the positions can be computed once per buffer
	float tta = sec*speedMult*M_2PI + M_2PI;
	float x = 12.0*cos(tta);
	float y = 12.0*sin(tta);
	float z = 4.0*sin(2.8 * tta);

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


//Converts spatial coordinate system to Allocore's OpenGL coordinate system
Vec3d convertCoords(Vec3d v){
	Vec3d returnVector;
	returnVector.x = v.x;
	returnVector.y = v.z;
	returnVector.z = -v.y;
	return returnVector;
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
		g.translate(convertCoords(srcpos));
		g.scale(0.1);
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
			g.scale(0.02 + 0.04 * peak * 30);

//			int chan = sChannels[i];
			// mFont.write(mText,to_string(chan));
			// mFont.texture().bind();
			// g.draw(mText);
			// mFont.texture().unbind();
//			mFont.render(g, to_string(chan));

			g.color(1);
            g.polygonLine();
			g.draw(mPoly);
			g.popMatrix();
		}
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
//    AudioDevice dev(12);
	AudioIO audioIO;
    audioIO.init(audioCB, nullptr, BLOCK_SIZE, 44100, outputChannels, 0);
//	audioIO.device(dev);


    panner->prepare(audioIO);
    audioIO.open();
	audioIO.start();

	MyApp().start();
	getchar();

	audioIO.stop();
	delete mPeaks;

	return 0;
}



