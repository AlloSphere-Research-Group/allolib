#include "al/core.hpp"
#include "al/util/imgui/al_Imgui.hpp"

using namespace al;

struct my_app : App
{
  float grayscale = 1;;
  Color clear_color {0, 0, 0};
  bool show_gui = true;
  Mesh m;

  void onCreate() override {
    initIMGUI();
    addSphere(m);
    nav().pos(0, 0, 10);
    nav().setHome();
  }

  void onAnimate(double dt) override {
    // pass show_gui for use_input param to turn off interactions
    // when not showing gui
    beginIMGUI_minimal(show_gui);
    navControl().active(!imgui_is_using_input());
  }

  void onDraw(Graphics& g) override {

    // Edit 3 floats representing a color
    ImGui::ColorEdit3("clear color", clear_color.components);
    ImGui::SliderFloat("grayscale", &grayscale, 0, 1);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    g.clear(clear_color);
    g.color(grayscale);
    g.draw(m);

    endIMGUI_minimal(show_gui);
  }

  void onKeyDown(const Keyboard& k) override {
    if (k.key() == 'g') {
      show_gui = !show_gui;
    }
  }

  void onExit() override {
    shutdownIMGUI();
  }
};

int main()
{
  my_app app;
  app.start();
}