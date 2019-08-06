#include "al/app/al_App.hpp"
#include "al/sound/al_SoundFile.hpp"
#include <iostream>
#include <vector>

using namespace al;

struct MyApp : App
{
  SoundFile soundFile;
  SoundFilePlayer player;
  std::vector<float> buffer;

  void onCreate() {
    player.soundFile = &soundFile;
    player.loop = true;
    player.pause = false;
    soundFile.open("data/count.wav");
    std::cout << "sampleRate: " <<  soundFile.sampleRate << std::endl;
    std::cout << "channels: " <<  soundFile.channels << std::endl;
    std::cout << "frameCount: " <<  soundFile.frameCount << std::endl;
  }

  void onDraw(Graphics& g) {
    g.clear(1, 0, 0);
  }

  void onSound(AudioIOData& io) {
    int frames = (int)io.framesPerBuffer();
    int channels = soundFile.channels;
    int bufferLength = frames * channels;
    if ((int)buffer.size() < bufferLength) {
      buffer.resize(bufferLength);
    }
    player.getFrames(frames, buffer.data(), (int)buffer.size());
    int second = (channels < 2)? 0 : 1;
    while (io()) {
      int frame = (int)io.frame();
      int idx = frame * channels;
      io.out(0) = buffer[idx];
      io.out(1) = buffer[idx + second];
    }
  }
};

int main() {
  MyApp app;
  app.initAudio(44100, 512, 2, 0);
  app.start();
}
