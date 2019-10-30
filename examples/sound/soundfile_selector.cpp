#include <iostream>
#include <string>
#include <vector>

#include "al/app/al_App.hpp"
#include "al/io/al_Imgui.hpp"
#include "al/sound/al_SoundFile.hpp"
#include "al/ui/al_FileSelector.hpp"

using namespace al;

// Sound file reading

struct MyApp : App {
  SoundFilePlayerTS playerTS;
  std::vector<float> buffer;
  bool loop = true;
  FileSelector selector{"", [](std::string name) {
                          if (name.substr(name.size() - 4) == ".wav")
                            return true;
                          else
                            return false;
                        }};

  void onCreate() override { imguiInit(); }

  void onDraw(Graphics& g) override {
    imguiBeginFrame();

    ImGui::Begin("control window");
    if (ImGui::Button("Select File")) {
      selector.start(File::currentPath() + "\\data");
    }
    if (ImGui::Button("play")) {
      playerTS.setPlay();
    }
    if (ImGui::Button("pause")) {
      playerTS.setPause();
    }
    ImGui::End();

    if (selector.isActive()) {
      ImGui::SetNextWindowSize({200, 200}, ImGuiCond_FirstUseEver);
      ImGui::Begin("File selector");
      if (selector.drawFileSelector()) {
        auto items = selector.getSelection();
        audioIO().stop();
        if (items.count() > 0) {
          auto filepath = items[0].filepath();
          if (playerTS.open(filepath.c_str())) {
            std::cout << "sampleRate: " << playerTS.soundFile.sampleRate
                      << std::endl;
            std::cout << "channels: " << playerTS.soundFile.channels
                      << std::endl;
            std::cout << "frameCount: " << playerTS.soundFile.frameCount
                      << std::endl;
            playerTS.setLoop();
            playerTS.setPlay();

            audioIO().framesPerSecond(playerTS.soundFile.sampleRate);
            audioIO().start();
          } else {
            std::cerr << "Error opening file: " << filepath << std::endl;
          }
        }
      }
      ImGui::End();
    }

    imguiEndFrame();
    g.clear(0, 0, 0);
    imguiDraw();
  }

  void onSound(AudioIOData& io) override {
    int frames = (int)io.framesPerBuffer();
    if (playerTS.soundFile.data.size() == 0) {
      // Ignore callback if no audio file
      return;
    }
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
  app.start();
}
