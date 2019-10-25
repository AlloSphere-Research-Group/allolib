#include <iostream>
#include <vector>

#include "al/app/al_App.hpp"
#include "al/sound/al_SoundFile.hpp"

using namespace al;

// Sound file reading streaming from disk.

struct MyApp : App {
  SoundFileStreaming player;
  std::vector<float> buffer;
  bool loop = true;

  void onInit() override {
    const char name[] = "data/count.wav";
    if (!player.open(name)) {
      std::cerr << "File not found: " << name << std::endl;
      quit();
    }
    std::cout << "sampleRate: " << player.sampleRate() << std::endl;
    std::cout << "channels: " << player.numChannels() << std::endl;
    std::cout << "frameCount: " << player.totalFrames() << std::endl;

    int channels = player.numChannels();
    int frames = (int)audioIO().framesPerBuffer();
    int bufferLength = frames * channels;
    // Prepare buffer to write samples from file to.
    if ((int)buffer.size() < bufferLength) {
      buffer.resize(bufferLength);
    }
  }

  void onSound(AudioIOData& io) override {
    int channels = player.numChannels();
    int second = (channels < 2) ? 0 : 1;  // buffer stride

    // Read interleaved frames into buffer
    player.getFrames(io.framesPerBuffer(), buffer.data());
    while (io()) {
      int frame = (int)io.frame();
      uint64_t idx = frame * channels;
      io.out(0) = buffer[idx];
      io.out(1) = buffer[idx + second];
    }
  }
};

int main() {
  MyApp app;
  app.configureAudio(44100, 512, 2, 0);
  app.start();
  return 0;
}
