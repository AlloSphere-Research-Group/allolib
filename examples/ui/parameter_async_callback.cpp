
#include <memory>
#include <thread>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ParameterGUI.hpp"

using namespace al;

struct MyApp : public App {
  Parameter x{"X", "", 0, "", -2.0, 2.0};
  float processedValue{0.0f};
  bool mRunning{true};

  std::unique_ptr<std::thread> valueCallbackThread;

  void onInit() override {
    // Register a callback for 'x' Parameter
    // For this simple example, just copy the parameter's value to the internal
    // variable
    x.registerChangeCallback([&](float value) { processedValue = value; });
    x.setSynchronousCallbacks(false);

    // Start a thread to run the parameters callbacks fpr 'x'
    // The parameter callbacks for 'x' will be called at most once per second
    valueCallbackThread = std::make_unique<std::thread>([&]() {
      while (mRunning) {
        x.processChange();
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
      std::cout << "Thread done." << std::endl;
    });
  }

  void onCreate() override {
    imguiInit();
    // Disable mouse nav to avoid naving while changing gui controls.
    navControl().useMouse(false);
  }

  void onAnimate(double /*dt*/) override {
    imguiBeginFrame();

    // The ParameterGUI class provides static functions to assist drawing
    // parameters and GUIs
    // Each beginPanel()/endPanel() pair creates a separate "window"
    ParameterGUI::beginPanel("Position Control");
    // The ParameterGUI::drawParameter() can take any parameter and will
    // draw and appropriate GUI
    ParameterGUI::drawParameter(&x);

    ImGui::Text("current: %f", processedValue);
    ImGui::Separator();
    ParameterGUI::endPanel();
    imguiEndFrame();
  }

  virtual void onDraw(Graphics &g) override {
    g.clear(0);

    // draw the GUI
    imguiDraw();
  }

  void onExit() override {
    imguiShutdown();
    mRunning = false;
    valueCallbackThread->join();
  }
};

int main() {
  MyApp app;
  app.title("Parameter GUI");
  app.start();
  return 0;
}
