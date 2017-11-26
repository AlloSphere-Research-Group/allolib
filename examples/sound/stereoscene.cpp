// AudioScene example using the Stereo panner class
// By Andrés Cabrera mantaraya36@gmail.com
// March 7 2017

#include <iostream>


#include "al/core.hpp"
#include "al/core/sound/al_StereoPanner.hpp"
#include "al/core/sound/al_AudioScene.hpp"

#define BLOCK_SIZE 256
#define SAMPLE_RATE 44100

using namespace al;

static SoundSource soundSource;
static AudioScene scene(BLOCK_SIZE);
static Listener *listener;
static StereoPanner *panner;
static SpeakerLayout speakerLayout;

class MyApp : public App {
	double mX, mY;
	Light mLight;
	// Sound variables
	float mEnvelope;
	Mesh m;

public:

	void onCreate() override {
		// Make a shape that will mark where the sound comes from.
		addTetrahedron(m, 0.2);
		m.generateNormals();

		// Audio
		mEnvelope = 0.3;
		speakerLayout = StereoSpeakerLayout();
		panner = new StereoPanner(speakerLayout);
		listener = scene.createListener(panner);
		scene.addSource(soundSource);
	}

	void onSound(AudioIOData &io) override {

		while(io()) {
			mEnvelope *= 0.9995;
			float noise = rnd::gaussian() * mEnvelope;
			if (mEnvelope < 0.0001) { mEnvelope = 0.3; }
			soundSource.writeSample(noise);
//			io.out(0) = noise;
		}
		scene.render(io);
	}

	void onDraw(Graphics &g) override {
		// Update the listener position from the visual scene's nav()
		listener->pose(nav());
//		std::cout << listener->pose().x() << "," << listener->pose().z() << std::endl;
//		std::cout << soundSource.pos().x << "," << soundSource.pos().z << std::endl;

		g.clear(0);

		g.depthTesting(true);
		g.lighting(true);
		g.light(mLight);
		g.color(1);
		
		g.pushMatrix();
		// Use the mouse Y position for distance
		// Scale the mouse pixel position to the current window size
		g.translate(mX, 0.0, mY);
		g.draw(m);
		g.popMatrix();
	}

	void onMouseMove(const Mouse &m) override {
		// Update mouse position
		mX = m.x()/320.0 - 1.0;
		mY = 4 -m.y()/40.0;
		// Sound source position is set whenever the mouse is moved
		soundSource.pos(mX, 0.0, mY);
	}

};

int main(int argc, char *argv[])
{
	MyApp app;
	app.dimensions(640, 480);
	app.title("Stereo Audio Scene");
	app.fps(30);
	app.initAudio(SAMPLE_RATE, BLOCK_SIZE, 2, 0);
	app.start();
	return 0;
}

