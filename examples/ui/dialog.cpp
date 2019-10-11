
#include "al/app/al_App.hpp"
#include "al/ui/al_Dialog.hpp"
#include "al/ui/al_ParameterGUI.hpp"

using namespace al;
using namespace std;

struct MyApp : public App {
  Dialog queryDialog{"Question", "How are you?",
                     Dialog::Buttons::AL_DIALOG_BUTTON_OK |
                         Dialog::Buttons::AL_DIALOG_BUTTON_CANCEL};

  std::string displayText = "press space for dialog.";

  void onCreate() override { imguiInit(); }

  void onDraw(Graphics &g) override {
    g.clear(0.5);

    imguiBeginFrame();

    ParameterGUI::beginPanel("Dialog Example");
    ImGui::Text("%s", displayText.c_str());
    ParameterGUI::endPanel();

    queryDialog.draw(g);

    imguiEndFrame();
    imguiDraw();
  }

  bool onKeyDown(const Keyboard &k) override {
    if (k.key() == ' ') {
      // When you start a dialog you can register a callback
      // function that will be called when the dialog exits
      queryDialog.start([&](Dialog::DialogResult ret) {
        if (ret == Dialog::DialogResult::AL_DIALOG_OK) {
          displayText = "Dialog returned OK ";
        } else if (ret == Dialog::DialogResult::AL_DIALOG_CANCEL) {
          displayText = "Dialog returned CANCEL ";
        }
      });
    }
    return true;
  }
};

int main(int argc, char *argv[]) {
  MyApp app;
  app.start();
  return 0;
}
