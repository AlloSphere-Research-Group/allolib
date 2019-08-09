#include <iostream>
#include <vector>
#include "al/app/al_App.hpp"
#include "al/io/al_Imgui.hpp"
#include "al/sound/al_SoundFile.hpp"

using namespace al;

// Sound file reading

struct MyApp : App {
  SoundFilePlayerTS playerTS;
  std::vector<float> buffer;
  bool loop = true;

  void onInit() override {
    const char name[] = "data/count.wav";
    if (!playerTS.open(name)) {
      std::cerr << "File not found: " << name << std::endl;
      quit();
    }
    std::cout << "sampleRate: " << playerTS.soundFile.sampleRate << std::endl;
    std::cout << "channels: " << playerTS.soundFile.channels << std::endl;
    std::cout << "frameCount: " << playerTS.soundFile.frameCount << std::endl;
    playerTS.setLoop();
    playerTS.setPlay();
  }

  void onCreate() override { imguiInit(); }

  void onDraw(Graphics& g) override {
    imguiBeginFrame();
    ImGui::Begin("control window");
    if (ImGui::Button("play")) {
      playerTS.setPlay();
    }
    if (ImGui::Button("pause")) {
      playerTS.setPause();
    }
    if (ImGui::Button("toggle pause")) {
      playerTS.togglePause();
    }
    if (ImGui::Button("rewind")) {
      playerTS.setRewind();
    }
    if (ImGui::Checkbox("loop", &loop)) {
      if (loop) {
        playerTS.setLoop();
      } else {
        playerTS.setNoLoop();
      }
    }
    ImGui::End();
    imguiEndFrame();
    g.clear(0, 0, 0);
    imguiDraw();
  }

  void onSound(AudioIOData& io) override {
    int frames = (int)io.framesPerBuffer();
    int channels = playerTS.soundFile.channels;
    int bufferLength = frames * channels;
    if ((int)buffer.size() < bufferLength) {
      buffer.resize(bufferLength);
    }
    playerTS.getFrames(frames, buffer.data(), (int)buffer.size());
    int second = (channels < 2) ? 0 : 1;
    while (io()) {
      int frame = (int)io.frame();
      int idx = frame * channels;
      io.out(0) = buffer[idx];
      io.out(1) = buffer[idx + second];
    }
  }

  void onExit() override { imguiShutdown(); }
};

int main() {
  MyApp app;
  app.configureAudio(44100, 512, 2, 0);
  app.start();
}
