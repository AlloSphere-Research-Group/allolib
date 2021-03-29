#ifndef GRAPHICSDOMAIN_H
#define GRAPHICSDOMAIN_H

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <stack>
#include <vector>

#include "al/app/al_ComputationDomain.hpp"
#include "al/app/al_FPS.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/io/al_ControlNav.hpp"
#include "al/io/al_Window.hpp"

namespace al {

class GLFWOpenGLWindowDomain;

struct WindowSetupProperties {
  Window::Cursor cursor = Window::Cursor::POINTER;
  bool cursorVisible = true;
  Window::Dim dimensions{50, 50, 640, 480};
  Window::DisplayMode displayMode{Window::DEFAULT_BUF};
  bool fullScreen = false;
  std::string title = "Alloapp";
  bool vsync = true;
  bool decorated = true;
};

/**
 * @brief OpenGLGraphicsDomain class
 * @ingroup App
 *
 * This domain prepares a GLFW OpenGL domain.
 */
class OpenGLGraphicsDomain : public AsynchronousDomain, public FPS {
public:
  virtual ~OpenGLGraphicsDomain() {}

  // Domain functions
  bool init(ComputationDomain *parent = nullptr) override;
  bool start() override;
  bool stop() override;
  bool cleanup(ComputationDomain *parent = nullptr) override;

  void quit() { mShouldQuitApp = true; }
  bool shouldQuit() { return mShouldQuitApp || mSubDomainList.size() == 0; }

  bool running() { return mRunning; }

  /**
   * @brief Create a new window
   * @return the new window domain
   *
   * newWindow() must be called after domain has been initialized, as it also
   * initializes the window domain, which requires an opengl context.
   */
  std::shared_ptr<GLFWOpenGLWindowDomain> newWindow();

  void closeWindow(std::shared_ptr<GLFWOpenGLWindowDomain> windowDomain);

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

/**
 * @brief Domain for a GLFW window
 *
 * set the onNewFrame, preOnDraw, onDraw and postOnDraw to determine what is
 * drawn in the window.
 *
 * You can get mouse and keyboard events through the Window instance avaialable
 * through window()
 */
class GLFWOpenGLWindowDomain : public SynchronousDomain {
public:
  GLFWOpenGLWindowDomain();
  // Domain functions
  bool init(ComputationDomain *parent = nullptr) override;

  bool tick() override;

  bool cleanup(ComputationDomain *parent = nullptr) override;

  /**
   * Called once on every frame loop
   */
  std::function<void()> onNewFrame = [this]() {
    mNav.smooth(std::pow(0.0001, mTimeDrift));
    mNav.step(mTimeDrift * mParent->fps());
  };

  /**
   * Called once before every draw call, when view and context has been
   * prepared
   */
  std::function<void()> preOnDraw = [this]() {
    mGraphics->framebuffer(FBO::DEFAULT);
    mGraphics->viewport(0, 0, mWindow->fbWidth(), mWindow->fbHeight());
    mGraphics->resetMatrixStack();
    mGraphics->camera(mView);
    mGraphics->color(1, 1, 1);
  };

  /**
   * Set this function to determine what is drawn
   */
  std::function<void(Graphics &)> onDraw = [](Graphics &g) { g.clear(0.3); };

  /**
   * Called once after every draw call, when view and context has been
   * prepared
   */
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

  Nav mNav; // is a Pose itself and also handles manipulation of pose
  Viewpoint mView{mNav.transformed()}; // Pose with Lens and acts as camera
  NavInputControl mNavControl{mNav};   // interaction with keyboard and mouse
};

} // namespace al

#endif // GRAPHICSDOMAIN_H
