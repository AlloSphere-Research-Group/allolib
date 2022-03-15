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
#include "al/types/al_SingleRWRingBuffer.hpp"

namespace al {

class GLFWOpenGLWindowDomain;

/**
 * @brief OpenGLGraphicsDomain class
 * @ingroup App
 *
 * This domain prepares a GLFW OpenGL domain.
 */
class OpenGLGraphicsDomain : public AsynchronousDomain, public FPS {
public:
  OpenGLGraphicsDomain();
  virtual ~OpenGLGraphicsDomain() {}

  // Domain functions
  bool init(ComputationDomain *parent = nullptr) override;
  bool start() override;
  bool stop() override;
  bool cleanup(ComputationDomain *parent = nullptr) override;

  void quit() { mShouldStopDomain = true; }
  bool shouldStop() { return mShouldStopDomain; }

  bool running() { return mRunning; }

  bool addSubDomain(std::shared_ptr<SynchronousDomain> subDomain,
                    bool prepend = false) override;
  bool removeSubDomain(std::shared_ptr<SynchronousDomain> subDomain) override;

  template <class DomainType>
  std::shared_ptr<DomainType> newSubDomain(bool prepend = false) {
    auto newDomain = std::make_shared<DomainType>();
    if (!dynamic_cast<SynchronousDomain *>(newDomain.get())) {
      // Only Synchronous domains are allowed as subdomains
      throw std::runtime_error(
          "Subdomain must be a subclass of SynchronousDomain");
    }
    addSubDomain(newDomain, prepend);
    return newDomain;
  }

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
  Window::WindowSetupProperties nextWindowProperties;

  std::function<void(void)> onCreate = []() {};
  std::function<void()> onExit = []() {};

  // Virtual functions to override
  virtual void preOnCreate() {}

  virtual void postOnExit() {}

  bool registerObject(void *object) override;

  bool unregisterObject(void *object) override;

protected:
  static void processDomainAddRemoveQueues(OpenGLGraphicsDomain *domain);
  static void domainThreadFunction(OpenGLGraphicsDomain *domain);

private:
  bool initPrivate();
  bool startPrivate();
  bool tick();
  bool stopPrivate();
  bool cleanupPrivate();

  std::mutex mSubDomainInsertLock;
  std::condition_variable mSubDomainInsertSignal;
  std::pair<std::shared_ptr<SynchronousDomain>, bool> mSubdomainToInsert;

  std::mutex mSubDomainRemoveLock;
  std::condition_variable mSubDomainRemoveSignal;
  std::shared_ptr<SynchronousDomain> mSubdomainToRemove;

  enum class CommandType { START, STOP, CLEANUP, NONE };
  std::mutex mDomainSignalLock;

  std::mutex mDomainCommandLock;
  std::condition_variable mDomainCommandSignal;
  CommandType mDomainCommand;
  bool mCommandResult;

  std::atomic<bool> mDomainStartSpinLock;
  std::unique_ptr<std::thread> mDomainThread;

  std::atomic<bool> mShouldStopDomain{false};
  bool mRunning{false};

  std::vector<GPUObject *> mObjects;
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
  std::function<void(Graphics &)> onDraw = [](Graphics &g) { g.clear(0.3f); };

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
