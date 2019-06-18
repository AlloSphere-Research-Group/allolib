#include <memory>
#include <iostream>

#include "al/util/ui/al_ControlGUI.hpp"

#include "al_ext/distributed/al_App.hpp"
#include "al/core/graphics/al_Shapes.hpp"

#include "Gamma/Oscillator.h"

using namespace al;


class MyApp: public BaseCompositeApp {
public:
  ParameterBool mVr {"VR", "", 0.0};
  ControlGUI gui;

  void onCreate() {
    mVr.registerChangeCallback([&](float value) {
      if (value != 0.0) {
        enableVR();
        setOpenVRDrawFunction(std::bind(&MyApp::drawVr, this, std::placeholders::_1));
      } else {
        disableVR();
      }
    });

#ifdef AL_EXT_OPENVR
    // Only show VR button if there is support for OpenVR
    gui << mVr;
#endif
    gui.init();
  }

  void drawScene(Graphics &g) {
    // This function is used by both the VR and the desktop draw calls.

    Mesh cube;
    addCube(cube);
    cube.primitive(Mesh::LINE_LOOP);

    g.color(1.0);
    g.draw(cube);
  }

  void drawVr(Graphics &g) {
    // HMD scene will have blueish background
    g.clear(0.0, 0.3, 0);
    drawScene(g);
  }

  void onDraw(Graphics &g) override {
    // Dekstop scene will have reddish background and gui.
    g.clear(0.3, 0, 0);
    drawScene(g);
    gui.draw(g);
  }

};

int main(int argc, char *argv[])
{
  MyApp app;
  app.start();


  return 0;
}
