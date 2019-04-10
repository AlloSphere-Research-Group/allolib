#include <memory>

#include "al/soundfile/al_SoundfileBuffered.hpp"
#include "al/core/app/al_App.hpp"

using namespace al;

struct MyApp : public App {

    SoundFileBuffered soundFile;
    std::unique_ptr<float []> buffer;

    void onInit() {
        soundFile.open("data/count.wav");
        buffer = std::make_unique<float []>(audioIO().framesPerBuffer() * soundFile.channels());
    }

    void onSound(AudioIOData &io) override {
        // The sound file can copy directly to the output buffer
        soundFile.read(buffer.get(), io.framesPerBuffer());
        float *sfBuffer = buffer.get();
        while(io()) {
            io.out(0) = *sfBuffer;
            // If soundfile had two channels, you could read it with:
//            io.out(1) = *(sfBuffer + 1);
            sfBuffer += soundFile.channels(); // advance pointer to next audio frame
        }
    }

};

int main(int argc, char *argv[]) {
    MyApp app;
    app.initAudio();
    app.start();
    return 0;
}
