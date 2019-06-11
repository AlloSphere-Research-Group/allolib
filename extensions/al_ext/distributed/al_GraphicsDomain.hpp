#ifndef GRAPHICSDOMAIN_H
#define GRAPHICSDOMAIN_H

#include <stack>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>
#include <cassert>

#include "al_ComputationDomain.hpp"

#include "Gamma/Domain.h"
#include "al/core/app/al_WindowApp.hpp"
#include "al/core/io/al_ControlNav.hpp"

namespace al {

class GraphicsDomain : public AsynchronousDomain, public gam::Domain
{
public:

  virtual ~GraphicsDomain() {}

  // Domain functions
  bool initialize(ComputationDomain *parent = nullptr) override;
  bool start() override;
  bool stop() override;
  bool cleanup(ComputationDomain *parent = nullptr) override;

//  void onKeyDown(Keyboard const& k) {}
//  void onKeyUp(Keyboard const& k) {}
//  void onMouseDown(Mouse const& m) {}
//  void onMouseUp(Mouse const& m) {}
//  void onMouseDrag(Mouse const& m) {}
//  void onMouseMove(Mouse const& m) {}
//  void onResize(int w, int h) {}
//  void onVisibility(bool v) {}

  std::function<void(void)> onInit = [](){};
  std::function<void(void)> onCreate = [](){};
  std::function<void(double dt)> onAnimate = [](double){};
  std::function<void(Graphics &)> onDraw = [](Graphics &){};
  std::function<void()> onExit = [](){};

  // Virtual functions to override

  virtual void preOnCreate() {
    app.append(mNavControl);
    app.mGraphics.init();
  }


  virtual void preOnAnimate(double dt) {
      mNav.smooth(std::pow(0.0001, dt));
      mNav.step(dt * app.fps());
  }

  virtual void preOnDraw() {
      app.mGraphics.framebuffer(FBO::DEFAULT);
      app.mGraphics.viewport(0, 0, app.fbWidth(), app.fbHeight());
      app.mGraphics.resetMatrixStack();
      app.mGraphics.camera(mView);
      app.mGraphics.color(1, 1, 1);
  }

  virtual void postOnDraw() {
    //
  }

  virtual void postOnExit() {
    //
  }

  Graphics &graphics() { return app.mGraphics;}

private:
  WindowApp app;
  Nav mNav; // is a Pose itself and also handles manipulation of pose
  Viewpoint mView {mNav.transformed()};  // Pose with Lens and acts as camera
  NavInputControl mNavControl {mNav}; // interaction with keyboard and mouse
};
}

#endif // GRAPHICSDOMAIN_H
