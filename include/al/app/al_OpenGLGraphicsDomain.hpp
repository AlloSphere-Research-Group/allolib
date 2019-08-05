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
#include "al/app/al_WindowApp.hpp"
#include "al/io/al_ControlNav.hpp"

namespace al {

class OpenGLWindowDomain;

class OpenGLGraphicsDomain : public AsynchronousDomain, public gam::Domain
{
public:

  virtual ~OpenGLGraphicsDomain() {}

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

  std::shared_ptr<OpenGLWindowDomain> newWindow() {
    auto newWindowDomain = newSubDomain<OpenGLWindowDomain>();
    return newWindowDomain;
  }

  void closeWindow(std::shared_ptr<OpenGLWindowDomain> windowDomain) {
    removeSubDomain(std::static_pointer_cast<SynchronousDomain>(windowDomain));
  }

  std::function<void(void)> onCreate = [](){};
  std::function<void(Graphics &)> onDraw = [](Graphics &){};
  std::function<void()> onExit = [](){};

  std::function<void(Keyboard const&)> onKeyDown  = [](Keyboard const&){};
  std::function<void(Keyboard const&)> onKeyUp = [](Keyboard const&){};
  std::function<void(Mouse const&)> onMouseDown = [](Mouse const&){};
  std::function<void(Mouse const&)> onMouseUp  = [](Mouse const&){};
  std::function<void(Mouse const&)> onMouseDrag  = [](Mouse const&){};
  std::function<void(Mouse const&)> onMouseMove  = [](Mouse const&){};
  std::function<void(Mouse const&)> onMouseScroll  = [](Mouse const&){};

  // Virtual functions to override

  virtual void preOnCreate() {
    app.append(mNavControl);
    app.mGraphics.init();
  }

  virtual void onNewFrame() {
      mTimeDrift = app.dt_sec();
      mNav.smooth(std::pow(0.0001, mTimeDrift));
      mNav.step(mTimeDrift * app.fps());
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

  bool running() {
    return mRunning;
  }

  Graphics &graphics() { return app.mGraphics;}

  NavInputControl &navControl() {return mNavControl;}

  // TODO change WindowApp to OpenGLWindowDomain
  WindowApp app;

  Nav mNav; // is a Pose itself and also handles manipulation of pose
  Viewpoint mView {mNav.transformed()};  // Pose with Lens and acts as camera
  NavInputControl mNavControl {mNav}; // interaction with keyboard and mouse

private:

  bool mRunning {false};
};

class OpenGLWindowDomain : public SynchronousDomain {
public:
  // Domain functions
  bool initialize(ComputationDomain *parent = nullptr) override;

  bool tick() override;

  bool cleanup(ComputationDomain *parent = nullptr) override;

  std::function<void()> preOnDraw = [this]() {
    mGraphics->framebuffer(FBO::DEFAULT);
    mGraphics->viewport(0, 0, mWindow.fbWidth(), mWindow.fbHeight());
    mGraphics->resetMatrixStack();
    mGraphics->camera(mView);
    mGraphics->color(1, 1, 1);

  };

  std::function<void(Graphics &)> onDraw = [](Graphics &){};

  std::function<void()> postOnDraw = []() {};

  Viewpoint& view() { return mView; }
  const Viewpoint& view() const { return mView; }

  Nav& nav() { return mNav; }
  // Nav& nav() { return mNav; }
  const Nav& nav() const { return mNav; }

  Window &window() {return mWindow;}

private:
  Window mWindow;
  Graphics *mGraphics {nullptr};

  Nav mNav; // is a Pose itself and also handles manipulation of pose
  Viewpoint mView {mNav.transformed()};  // Pose with Lens and acts as camera

};


}

#endif // GRAPHICSDOMAIN_H
