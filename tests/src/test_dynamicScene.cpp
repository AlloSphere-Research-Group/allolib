
#include "catch.hpp"

#include "al/util/scene/al_DynamicScene.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/sound/al_Ambisonics.hpp"
#include "al/io/al_AudioIOData.hpp"

using namespace al;

class Voice : public PositionedVoice {
    virtual void onProcess(AudioIOData& io) override {
        float outvalue = bufferCounter++;
        while(io()) {
            io.out(0) = outvalue;
            outvalue += 0.5;
        }
    }

    float bufferCounter = 1;
};

TEST_CASE( "Dynamic Scene Stereo Spatilaization" ) {

    const int numChannels = 2;
    AudioIOData audioData;
    audioData.framesPerBuffer(16);
    audioData.framesPerSecond(44100);
    audioData.channelsIn(0);
    audioData.channelsOut(numChannels);

    DynamicScene scene;
    scene.prepare(audioData);

    SpeakerLayout layout = StereoSpeakerLayout();

    std::shared_ptr<StereoPanner> s = scene.setSpatializer<StereoPanner>(layout);
    // configure spatializer
//    std::shared_ptr<AmbisonicsSpatializer> s = scene.setSpatializer<AmbisonicsSpatializer>(layout);
//    s->configure(2, 1, 1); // dim, order, flavor

    Voice *newVoice = scene.getVoice<Voice>();
    newVoice->useDistanceAttenuation(false);
    scene.triggerOn(newVoice);

    // Listener in origin looking into neg z-axis
    scene.listenerPose().pos() = Vec3d(0,0,0);
    scene.listenerPose().faceToward(Vec3d(0,0, -4));

    newVoice->pose().pos() = Vec4d(1.0, 0.0, 0.0); // hard right

    audioData.zeroOut(); // Buffers are not cleared by default
    scene.render(audioData);

	float *bufl = audioData.outBuffer(0);
    float *bufr = audioData.outBuffer(1);
    for (int i = 0; i < 16; i++) {
        REQUIRE(*bufl++ < 1e-15);
        REQUIRE(*bufr++ == 1 + (0.5 * i));
    }

    newVoice->pose().pos() = Vec4d(-1.0, 0.0, 0.0); // hard left

    audioData.zeroOut(); // Buffers are not cleared by default
    scene.render(audioData);

    bufl = audioData.outBuffer(0);
    bufr = audioData.outBuffer(1);
    for (int i = 0; i < 16; i++) {
        REQUIRE(*bufl++ == 2 + (0.5 * i));
        REQUIRE(*bufr++ < 1e-15);
    }
}

TEST_CASE( "Dynamic Scene Ambisonics Spatilaization" ) {
    const int numChannels = 8;
    const int fpb = 4;
    AudioIOData audioData;
    audioData.framesPerBuffer(fpb);
    audioData.framesPerSecond(44100);
    audioData.channelsIn(0);
    audioData.channelsOut(numChannels);

    DynamicScene scene;
    scene.prepare(audioData);

    SpeakerLayout layout = SpeakerRingLayout<numChannels>();

    std::shared_ptr<AmbisonicsSpatializer> s = scene.setSpatializer<AmbisonicsSpatializer>(layout);
    // configure spatializer
    s->configure(3, 3, 1); // dim, order, flavor

    Voice *newVoice = scene.getVoice<Voice>();
    newVoice->useDistanceAttenuation(false);
    scene.triggerOn(newVoice);

    // Listener in origin looking into neg z-axis
    scene.listenerPose().pos() = Vec3d(0,0,0);
    scene.listenerPose().faceToward(Vec3d(0,0, -4));

    // Place voice in front
    newVoice->pose().pos()[2] = -4.0;

    s->print();
    audioData.zeroOut(); // Buffers are not cleared by default
    scene.render(audioData);


	for (int spkr = 0; spkr < layout.numSpeakers(); spkr++) {
        float *buf = audioData.outBuffer(spkr);
		for (int i = 0; i < fpb; i++) {

            std::cout << spkr << ":" << i << "   :  " << *buf++ << std::endl;
//			REQUIRE(speakerSignals[i] > speakerSignals[spkr * bufferSize + 1]);
		}
        std::cout << "-----" << std::endl;
	}
}

