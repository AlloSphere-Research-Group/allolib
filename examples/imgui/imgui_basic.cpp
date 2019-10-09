#include "al/app/al_App.hpp"
#include "al/io/al_Imgui.hpp"

struct MyApp : al::App {
  void onCreate() override { al::imguiInit(); }

  void onAnimate(double dt) override {
    // start writing ui
    al::imguiBeginFrame();

    // do plain imgui stuff
    ImGui::Begin("my window");
    ImGui::Text("text widget");
    ImGui::End();

    // finish writing ui
    al::imguiEndFrame();
  }

  void onDraw(al::Graphics& g) override {
    g.clear(0);
    // actual ui drawing
    al::imguiDraw();
  }

  void onExit() override { al::imguiShutdown(); }
};

int main(int argc, char* argv[]) {
  MyApp app;
  app.dimensions(640, 480);
  app.start();
  return 0;
}
