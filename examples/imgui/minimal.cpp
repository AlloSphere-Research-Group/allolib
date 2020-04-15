#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/io/al_Imgui.hpp"

using namespace al;

struct MyApp : App {
  float grayscale = 1;
  Color clear_color{0, 0, 0};
  bool show_gui = true;
  Mesh m;

  void onCreate() override {
    imguiInit();
    addSphere(m);
    nav().pos(0, 0, 10);
    nav().setHome();
  }

  void onAnimate(double dt) override {
    navControl().active(!isImguiUsingInput());

    if (show_gui) {
      imguiBeginFrame();

      ImGui::SetNextWindowSize(ImVec2(0, 0));
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGuiWindowFlags flags = 0;
      flags |= ImGuiWindowFlags_NoTitleBar;
      flags |= ImGuiWindowFlags_NoResize;
      flags |= ImGuiWindowFlags_NoMove;
      flags |= ImGuiWindowFlags_NoScrollbar;
      flags |= ImGuiWindowFlags_NoCollapse;
      flags |= ImGuiWindowFlags_AlwaysAutoResize;
      flags |= ImGuiWindowFlags_NoBackground;
      ImGui::Begin("my window", NULL, flags);
      ImGui::ColorEdit3("clear color", clear_color.components);
      ImGui::SliderFloat("grayscale", &grayscale, 0, 1);
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();

      imguiEndFrame();
    }
  }

  void onDraw(Graphics &g) override {
    g.clear(clear_color);
    g.color(grayscale);
    g.draw(m);

    if (show_gui)
      imguiDraw();
  }

  bool onKeyDown(const Keyboard &k) override {
    if (k.key() == 'g') {
      show_gui = !show_gui;
    }
    return true;
  }

  void onExit() override { imguiShutdown(); }
};

int main() {
  MyApp app;
  app.start();
  return 0;
}
