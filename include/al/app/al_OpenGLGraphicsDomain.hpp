#ifndef GRAPHICSDOMAIN_H
#define GRAPHICSDOMAIN_H

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <stack>
#include <vector>

#include "al_ComputationDomain.hpp"

#include "al/app/al_WindowApp.hpp"
#include "al/io/al_ControlNav.hpp"

namespace al {

class GLFWOpenGLWindowDomain;

struct WindowSetupProperties {
  Window::Cursor cursor = Window::Cursor::POINTER;
  bool cursorVisible = true;
  Window::Dim dimensions {50, 50, 640, 480};
  Window::DisplayMode displayMode {Window::DEFAULT_BUF};
  bool fullScreen = false;
  std::string title = "Alloapp";
  bool vsync = true;
  bool decorated = true;
};

class OpenGLGraphicsDomain : public AsynchronousDomain, public FPS {
 public:
  virtual ~OpenGLGraphicsDomain() {}

  // Domain functions
  bool initialize(ComputationDomain *parent = nullptr) override;
  bool start() override;
  bool stop() override;
  bool cleanup(ComputationDomain *parent = nullptr) override;

  void quit() { mShouldQuitApp = true; }
  bool shouldQuit() { return mShouldQuitApp || mSubDomainList.size() == 0; }

  std::shared_ptr<GLFWOpenGLWindowDomain> newWindow();

  void closeWindow(std::shared_ptr<GLFWOpenGLWindowDomain> windowDomain) {
    removeSubDomain(std::static_pointer_cast<SynchronousDomain>(windowDomain));
  }

  // Next window details
  WindowSetupProperties nextWindowProperties;

  std::function<void(void)> onCreate = []() {};
  std::function<void()> onExit = []() {};

  // Virtual functions to override
  virtual void preOnCreate() {}

  virtual void postOnExit() {}

 private:
  std::atomic<bool> mShouldQuitApp{false};
  bool mRunning{false};
};

class GLFWOpenGLWindowDomain : public SynchronousDomain {
 public:
  GLFWOpenGLWindowDomain();
  // Domain functions
  bool initialize(ComputationDomain *parent = nullptr) override;

  bool tick() override;

  bool cleanup(ComputationDomain *parent = nullptr) override;

  std::function<void()> preOnDraw = [this]() {
    mGraphics->framebuffer(FBO::DEFAULT);
    mGraphics->viewport(0, 0, mWindow->fbWidth(), mWindow->fbHeight());
    mGraphics->resetMatrixStack();
    mGraphics->camera(mView);
    mGraphics->color(1, 1, 1);
  };

  virtual void onNewFrame() {
    mNav.smooth(std::pow(0.0001, mTimeDrift));
    mNav.step(mTimeDrift * mParent->fps());
  }

  std::function<void(Graphics &)> onDraw = [](Graphics &) {};

  std::function<void()> postOnDraw = []() {};

  Viewpoint &view() { return mView; }
  const Viewpoint &view() const { return mView; }

  Nav &nav() { return mNav; }
  // Nav& nav() { return mNav; }
  const Nav &nav() const { return mNav; }

  NavInputControl &navControl() { return mNavControl; }

  Window &window() { return *mWindow; }

  Graphics &graphics() { return *mGraphics; }

 private:
  std::unique_ptr<Window> mWindow;

  std::unique_ptr<Graphics> mGraphics;

  OpenGLGraphicsDomain *mParent;

  Nav mNav;  // is a Pose itself and also handles manipulation of pose
  Viewpoint mView{mNav.transformed()};  // Pose with Lens and acts as camera
  NavInputControl mNavControl{mNav};    // interaction with keyboard and mouse
};

}  // namespace al

#endif  // GRAPHICSDOMAIN_H
