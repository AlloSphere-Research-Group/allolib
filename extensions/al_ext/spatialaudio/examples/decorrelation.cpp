/*
  Decorrelation example
  by: Andres Cabrera
*/

#include <iostream>

#include "al/core/app/al_AudioApp.hpp"

#include "al_ext/spatialaudio/al_Decorrelation.hpp"

#include "Gamma/Noise.h"
#include "Gamma/Envelope.h"

/* This example will generate an exponentially decaying noise burst and
 * then produce 8 decorrelated copies of it using random phase all-pass
 * filters. It uses the Kendall method to produce the IRs, that consists
 * of generating a random phase spectrum and then doing an inverse FFT
 * to obtain the IR for that random phase filter.
 * In this example, every 3 bursts of noise, a new set of decorrelation IRs
 * is produced changing the maxjump parameter. As the maxjump parameter
 * decreases, the correlation between the IRs increases as there will be less
 * variation in the randomness of the phase, and the sound will become
 * "narrower".
*/

using namespace al;

class MyApp : public AudioApp{
public:
	float mMaxjump{M_PI};
	int mCounter;
	int mCounterTarget;

	// The decorrelation IR size is set to 1024
	Decorrelation decorrelation{1024};

	gam::NoisePink<> noise;
	gam::Decay<> env {48000};

	// The decorrelation object is initialized in the constructor
	
	// The input channel is set to -1 to indicate parallel or "many to many" mode
	// The number of channels is set to 8
	// And the last parameter "inputsAreBuses" is set to true to indicate
	// that inputs are buses instead of hardware input channels
	MyApp() 
	{
		audioIO().channelsBus(1); // Make 1 bus, we will write decorrelation input to it.

		// Define routing. Key to map are inputs, map value are the channel
		// indeces where decorrelated copies of the input are sent.
        map<uint32_t, vector<uint32_t>> routingMap = {{0, {0,1,2,3,4,5,6,7}}};

		decorrelation.configure(audioIO(), routingMap, true, 1000, mMaxjump);
		// You can append the decorrelation object to the AudioIO object
		// This will add the audio process callback to the AudioIO processing
		// after the user defined callback below
		audioIO().append(decorrelation);
		
		// Counter to force a generation of new decorrelation IRs every three seconds
		mCounterTarget = 3 * audioIO().framesPerSecond()/ audioIO().framesPerBuffer();
		mCounter = mCounterTarget; 
	}

	// Audio callback
	void onSound(AudioIOData& io) override {
		if (++mCounter >= mCounterTarget) { // Check if it's time to generate new IRs.
            map<uint32_t, vector<uint32_t>> routingMap = {{0, {0,1,2,3,4,5,6,7}}};
			decorrelation.configure(audioIO(), routingMap, true, 1000, mMaxjump);
			std::cout << "Max jump set to: " << mMaxjump << std::endl;
            mMaxjump *= 0.5;
			if (mMaxjump < 0.05) { mMaxjump = M_PI; } // Wrap max jump
			mCounter = 0;
		}
		while(io()){
			io.bus(0) = noise() * env(); // Generate noise bursts into first bus
			if (env.done()) { env.reset(); } // Retrigger envelope when done
		}
		// Decorrelation is performed without the need to add anything here
		// as the decorrelation object was "appended" to the AudioIO object.
	}
};


int main(){
	MyApp app;
	// 8 audio outputs
	app.initAudio(44100, 512, 8, 0);
	std::cout << "Press enter to quit" << std::endl;
	app.beginAudio();
	getchar();
	app.endAudio();
	return 0;
}
