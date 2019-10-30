#include <iostream>
#include <string>
#include <vector>

#include "al/app/al_App.hpp"
#include "al/io/al_Imgui.hpp"
#include "al/sound/al_SoundFile.hpp"
#include "al/ui/al_FileSelector.hpp"

using namespace al;

// This example shows how to use the file selector class

struct MyApp : App {
  FileSelector selector;
  // You can specify a root path (a path you can't go down beyond) and a
  // filtering function in the constructor of the selector
  //  FileSelector selector{"/Volumes/Data", [](std::string name) { return
  //  name.substr(name.size() - 4) == ".wav";}};
  std::string currentFile = "No file selected";

  void onCreate() override { imguiInit(); }

  void onDraw(Graphics& g) override {
    imguiBeginFrame();

    ImGui::Begin("File Selector");

    ImGui::Text("%s", currentFile.c_str());
    if (ImGui::Button("Select File")) {
      // When the select file button is clicked, the file selector is shown
      selector.start("");
    }
    // The file selector knows internally whether it should be drawn or not,
    // so you should always draw it. Check the return value of the draw function
    // to know if the user has selected a file through the file selector
    if (selector.drawFileSelector()) {
      auto selection = selector.getSelection();
      if (selection.count() > 0) {
        currentFile = selection[0].filepath();
      }
    }
    ImGui::End();
    imguiEndFrame();
    g.clear(0, 0, 0);
    imguiDraw();
  }

  void onExit() override { imguiShutdown(); }
};

int main() {
  MyApp app;
  app.start();
  return 0;
}
