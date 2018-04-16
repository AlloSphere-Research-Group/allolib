// based on dear imgui's example `examples/opengl3_example/main.cpp`
// (https://github.com/ocornut/imgui.git)

#include "al/core.hpp"
#include "al/util/imgui/al_Imgui.hpp"

using namespace al;

struct my_app : App
{
  float x = 0;
  Color clear_color {0, 0, 0};
  bool show_another_window = false;
  bool show_demo_window = false;
  int counter = 0;
  Mesh m;

  void onCreate() override {
    initIMGUI();
    addSphere(m);
    nav().pos(0, 0, 10);
    nav().setHome();
  }

  void onAnimate(double dt) override {
    // call beginIMGUI before everything, so info about gui status can be used
    // to update app state
    beginIMGUI();

    // don't nav if imgui's using inputs
    // ex) prevents camera rotation when mouse dragging scroll bar
    // nav is update after onAnimate and before onDraw.
    // so need to flag active/inactive here
    auto& io = ImGui::GetIO();
    bool using_gui = io.WantCaptureMouse | io.WantCaptureKeyboard
                                         | io.WantTextInput;
    navControl().active(!using_gui);
  }

  void onDraw(Graphics& g) override {

    {
      // Display some text (you can use a format string too)
      ImGui::Text("Hello, world!"); 
      // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::SliderFloat("x", &x, 0.0f, 1.0f);
      // Edit 3 floats representing a color
      ImGui::ColorEdit3("clear color", clear_color.components);
      // Edit bools storing our windows open/close state
      ImGui::Checkbox("Another Window", &show_another_window);
      ImGui::Checkbox("Demo Window", &show_demo_window);

      if (ImGui::Button("Button")) {
        // Buttons return true when clicked
        // (NB: most widgets return true when edited/activated)
        counter++;
      }
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }

    // 2. Show another simple window. In most cases you will use an explicit
    // Begin/End pair to name your windows.
    if (show_another_window) {
        ImGui::Begin("Another Window", &show_another_window);
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me")) {
          show_another_window = false;
        }
        ImGui::End();
    }

    // 3. Show the ImGui demo window. Most of the sample code is in
    // ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
    if (show_demo_window) {
      // Normally user code doesn't need/want to call this because positions are
      // saved in .ini file anyway. Here we just want to make the demo initial
      // state a bit more friendly!
      ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); 
      ImGui::ShowDemoWindow(&show_demo_window);
    }

    g.clear(clear_color);
    g.translate(x, 0, 0);
    g.color(1);
    g.draw(m);

    endIMGUI(); // after everything, actual rendering happens here
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