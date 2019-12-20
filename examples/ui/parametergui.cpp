
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ParameterGUI.hpp"

using namespace al;

// You can also create Parameter GUIs without the ControlGUI
// manager to use ImGUI more directly. This gives you better
// control over layout and style, but requires a bit more
// knowledge of how ImGUI works

struct MyApp : public App {
  Parameter x{"X", "", 0, "", -2.0, 2.0};
  Parameter y{"Y", "", 0, "", -2.0, 2.0};
  Parameter z{"Z", "", 0, "", -2.0, 2.0};

  ParameterColor color{"Color"};

  ImVec4 textColor = {1.0, 0.3f, 0.1f, 1.0};

  Mesh mMesh;

  void onCreate() override {
    addSphere(mMesh, 0.1);
    mMesh.primitive(Mesh::LINES);

    // You can set the display name for the parameter to be different to its
    // name
    z.displayName("Pos(Z)");

    nav() = Vec3d(0, 0, 2);
    // We must initialize ImGUI ourselves:
    imguiInit();
    // Disable mouse nav to avoid naving while changing gui controls.
    navControl().useMouse(false);
  }

  virtual void onDraw(Graphics &g) override {
    g.clear(0);
    g.pushMatrix();
    g.translate(x, y, z);
    g.color(color);
    g.draw(mMesh);
    g.popMatrix();

    // You are responsible for wrapping all your ImGUI code
    // between imguiBeginFrame() and imguiEndFrame()
    // Don't forget this or this will crash or not work!

    imguiBeginFrame();

    // The ParameterGUI class provides static functions to assist drawing
    // parameters and GUIs
    // Each beginPanel()/endPanel() pair creates a separate "window"
    ParameterGUI::beginPanel("Position Control");
    // The ParameterGUI::drawParameter() can take any parameter and will
    // draw and appropriate GUI
    ParameterGUI::drawParameter(&x);
    ParameterGUI::drawParameter(&y);
    ParameterGUI::drawParameter(&z);
    // The following are direct calls to ImGUI
    ImGui::TextColored(textColor, "Colored Text");
    ImGui::Separator();
    ImGui::ColorPicker3("Choose Color for text", (float *)&textColor,
                        ImGuiColorEditFlags_Uint8 |
                            ImGuiColorEditFlags_DisplayRGB |
                            ImGuiColorEditFlags_InputRGB);
    ParameterGUI::endPanel();

    // Specifying position and/or width makes the position/width fixed
    // A value of -1 in width or height sets automatic width
    ParameterGUI::beginPanel("Control Color", 5, 100, 150, -1);
    ParameterGUI::drawParameterMeta(&color);
    ParameterGUI::endPanel();

    imguiEndFrame();

    // Finally, draw the GUI
    imguiDraw();
  }

  void onExit() override { imguiShutdown(); }
};

int main(int argc, char *argv[]) {
  MyApp app;
  app.title("Parameter GUI");
  app.start();
  return 0;
}
