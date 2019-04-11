#include <memory>

#include "al/core/app/al_App.hpp"

#include "al_ext/soundfile/al_OutputRecorder.hpp"

#include "Gamma/Oscillator.h"

using namespace al;

struct MyApp : public App {

    OutputRecorder recorder;

    void onInit() {
        // Open the sound file for writing. This will be a two channel soundfile
        // The default is to use 16-bit int WAV files.
        recorder.start("output.wav", audioIO().framesPerSecond(), 2);
        // Append recorder to audio IO object. This will run the recorder
        // after running the onSound() function below
        audioIO().append(recorder);
    }

    void onSound(AudioIOData &io) override {
        while(io()) {
            // Copy inputs to outputs
            io.out(0) = io.in(0) * 0.3;
            io.out(1) = io.in(1) * 0.3;
        }
        // Output will be recorded to the file after this
    }

    void onExit() {
        recorder.close();
    }
};

int main(int argc, char *argv[]) {
    MyApp app;
    app.initAudio(AudioApp::IN_AND_OUT);
    app.start();
    return 0;
}
