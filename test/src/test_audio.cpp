#include "al/core/io/al_AudioIO.hpp"

using namespace al;

TEST_CASE( "Audio Device Enum" ) {
    std::cout << "RtAudio --------------------------" << std::endl;
    AudioDevice::printAll();
    std::cout << "Portaudio --------------------------" << std::endl;
    AudioDevice::printAll(AudioIOData::PORTAUDIO);
}

static void callback(AudioIOData &io) {
    static double phase = 0.0;
    static double phaseInc = M_2PI * 440.0 / io.framesPerSecond();
    while(io()) {
        io.out(0) = std::sin(phase)* 0.2;
        phase += phaseInc;
        if (phase > M_2PI) {
            phase -= M_2PI;
        }
    }
}

TEST_CASE( "Audio IO Object" ) {
    int userData = 5;
    std::cout << "RtAudio --------------------------" << std::endl;
    AudioIO audioIO;
    audioIO.init(callback, &userData, 64, 44100.0, 2, 2, AudioIO::RTAUDIO);
    audioIO.print();
    REQUIRE(audioIO.user<int>() == 5);
    REQUIRE(audioIO.open());
    REQUIRE(audioIO.start());
    al_sleep(0.5);
    REQUIRE(audioIO.stop());
    REQUIRE(audioIO.close());
    REQUIRE(audioIO.user<int>() == 5);

    std::cout << "Portaudio --------------------------" << std::endl;
    AudioIO audioIO2;
    audioIO2.init(callback, &userData, 64, 44100.0, 2, 2, AudioIOData::PORTAUDIO);
    audioIO2.print();
    REQUIRE(audioIO2.user<int>() == 5);
    REQUIRE(audioIO2.open());
    REQUIRE(audioIO2.start());
    al_sleep(0.5);
    REQUIRE(audioIO2.stop());
    REQUIRE(audioIO2.close());
    REQUIRE(audioIO2.user<int>() == 5);
}

TEST_CASE("Audio Channels/Virtual Channels") {
    AudioIO audioIO;
    audioIO.init(nullptr, nullptr, 256, 44100, 1, 1, AudioIOData::DUMMY);

    // Make sure parameters match those passed to constructor
    audioIO.open();
    REQUIRE(audioIO.framesPerBuffer() == 256);
    REQUIRE(audioIO.fps() == 44100);
    REQUIRE(audioIO.channelsOut() == 1);
    REQUIRE(audioIO.channelsIn() == 1);
    audioIO.close();

    // Test virtual channels
    int maxChansOut = AudioDevice::defaultOutput(AudioIOData::DUMMY).channelsOutMax();
    int maxChansIn  = AudioDevice::defaultInput(AudioIOData::DUMMY).channelsInMax();
    audioIO.channelsOut(maxChansOut + 1);
    audioIO.channelsIn (maxChansIn  + 1);
    audioIO.open();
    REQUIRE(audioIO.channelsOutDevice() == maxChansOut); // opened all hardware channels?
    REQUIRE(audioIO.channelsOut() == (maxChansOut+1)); // got our extra virtual channel?
    REQUIRE(audioIO.channelsInDevice() == maxChansIn); // opened all hardware channels?
    REQUIRE(audioIO.channelsIn() == (maxChansIn+1)); // got our extra virtual channel?
    audioIO.close();

    // Now RTAUDIO
    AudioIO audioIO1;
    audioIO1.init(nullptr, nullptr,256, 44100,  1, 1, AudioIOData::RTAUDIO);

    // Make sure parameters match those passed to constructor
    audioIO1.open();
    REQUIRE(audioIO1.framesPerBuffer() == 256);
    REQUIRE(audioIO1.fps() == 44100);
    REQUIRE(audioIO1.channelsOut() == 1);
    REQUIRE(audioIO1.channelsIn() == 1);
    audioIO1.close();

    // Test virtual channels
    maxChansOut = AudioDevice::defaultOutput(AudioIOData::RTAUDIO).channelsOutMax();
    maxChansIn  = AudioDevice::defaultInput(AudioIOData::RTAUDIO).channelsInMax();
    audioIO1.channelsOut(maxChansOut + 1);
    audioIO1.channelsIn (maxChansIn  + 1);
    audioIO1.open();
    REQUIRE(audioIO1.channelsOutDevice() == maxChansOut); // opened all hardware channels?
    REQUIRE(audioIO1.channelsOut() == (maxChansOut+1)); // got our extra virtual channel?
    REQUIRE(audioIO1.channelsInDevice() == maxChansIn); // opened all hardware channels?
    REQUIRE(audioIO1.channelsIn() == (maxChansIn+1)); // got our extra virtual channel?
    audioIO1.close();

    // Now PORTAUDIO
    AudioIO audioIO2;
    audioIO2.init(nullptr, nullptr, 256, 44100, 1, 1, AudioIOData::PORTAUDIO);

    // Make sure parameters match those passed to constructor
    audioIO2.open();
    REQUIRE(audioIO2.framesPerBuffer() == 256);
    REQUIRE(audioIO2.fps() == 44100);
    REQUIRE(audioIO2.channelsOut() == 1);
    REQUIRE(audioIO2.channelsIn() == 1);
    audioIO2.close();

    // Test virtual channels
    maxChansOut = AudioDevice::defaultOutput(AudioIOData::PORTAUDIO).channelsOutMax();
    maxChansIn  = AudioDevice::defaultInput(AudioIOData::PORTAUDIO).channelsInMax();
    audioIO2.channelsOut(maxChansOut + 1);
    audioIO2.channelsIn (maxChansIn  + 1);
    audioIO2.open();
    REQUIRE(audioIO2.channelsOutDevice() == maxChansOut); // opened all hardware channels?
    REQUIRE(audioIO2.channelsOut() == (maxChansOut+1)); // got our extra virtual channel?
    REQUIRE(audioIO2.channelsInDevice() == maxChansIn); // opened all hardware channels?
    REQUIRE(audioIO2.channelsIn() == (maxChansIn+1)); // got our extra virtual channel?
    audioIO.close();
}
