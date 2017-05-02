#include <cmath>

#include "al/core/io/al_AudioIO.hpp"
#include "al/core/system/al_Time.hpp"

using namespace al;

TEST_CASE( "Audio Device Enum" ) {
    std::cout << "RtAudio --------------------------" << std::endl;
    AudioDevice::printAll();
    std::cout << "Portaudio --------------------------" << std::endl;
    AudioDevice::printAll(AudioIOData::PORTAUDIO);
}

static void callback(AudioIOData &io) {
    static double phase = 0.0;
    static double phaseInc = M_2_PI * 440.0 / io.framesPerSecond();
    while(io()) {
        io.out(0) = std::sin(phase)* 0.2;
        phase += phaseInc;
        if (phase > M_2_PI) {
            phase -= M_2_PI;
        }
    }
}

TEST_CASE( "Audio IO Object" ) {
    int userData = 5;
    std::cout << "RtAudio --------------------------" << std::endl;
    AudioIO audioIO(64, 44100.0, callback, &userData, 2, 2, AudioIO::RTAUDIO);
    audioIO.print();
    REQUIRE(audioIO.user<int>() == 5);
    REQUIRE(audioIO.open());
    REQUIRE(audioIO.start());
    al_sleep(0.5);
    REQUIRE(audioIO.stop());
    REQUIRE(audioIO.close());
    REQUIRE(audioIO.user<int>() == 5);

    std::cout << "Portaudio --------------------------" << std::endl;
    AudioIO audioIO2(64, 44100.0, callback, &userData, 2, 2, AudioIOData::PORTAUDIO);
    audioIO2.print();
    REQUIRE(audioIO2.user<int>() == 5);
    REQUIRE(audioIO2.open());
    REQUIRE(audioIO2.start());
    al_sleep(0.5);
    REQUIRE(audioIO2.stop());
    REQUIRE(audioIO2.close());
    REQUIRE(audioIO.user<int>() == 5);
}
