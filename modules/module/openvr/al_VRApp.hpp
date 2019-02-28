#ifndef INCLUDE_VRRENDER_HPP
#define INCLUDE_VRRENDER_HPP

// #include "al/sphere/al_Perprojection.hpp"
// #include "al/sphere/al_SphereUtils.hpp"

#include "al/core/app/al_WindowApp.hpp"
#include "al/core/app/al_AudioApp.hpp"
#include "al/core/graphics/al_GLFW.hpp"
#include "al/core/graphics/al_Graphics.hpp"

#include "module/openvr/al_OpenVRWrapper.hpp"

#include <iostream>
#include <functional>

namespace al {

class VRApp : public WindowApp, public AudioApp, public osc::PacketHandler
{
public:
    VRApp() : WindowApp() {}
    // `WindowApp()` appends std window controls and itself as event handler
    virtual ~VRApp() {}

    Nav &nav() {return mNav;}
    // getters trying to match al::App interface
    virtual Graphics &graphics() { return mGraphics; }
    virtual const Graphics &graphics() const { return mGraphics; }
    virtual Lens& lens() { return mLens; }
    virtual const Lens& lens() const { return mLens; }
    virtual void pose(Pose const &p) { mPose = p; }
    virtual Pose &pose() { return mPose; }
    virtual Pose const &pose() const { return mPose; }

    // for user to override
    void onInit() override {}
    void onCreate() override {}
    void onAnimate(double dt) override {}
    void onDraw(Graphics &g) override {}
    void onExit() override {}
    void onKeyDown(Keyboard const &k) override {}
    void onKeyUp(Keyboard const &k) override {}
    void onMouseDown(Mouse const &m) override {}
    void onMouseUp(Mouse const &m) override {}
    void onMouseDrag(Mouse const &m) override {}
    void onMouseMove(Mouse const &m) override {}
    void onMessage(osc::Message &m) override {}
    void onResize(int w, int h) override {}
    void onVisibility(bool v) override {}

    // run app
    void start() override;
protected:
  Lens mLens;
  Pose mPose;
  Nav mNav;

  //openVR wrapper
  OpenVRWrapper *openVR;
};

inline void VRApp::start() {
  glfw::init(is_verbose);
  onInit();

  Window::create(is_verbose);
  mGraphics.init();

  openVR = new OpenVRWrapper();
  if(!openVR->init()) {
      std::cerr << "ERROR: OpenVR init returned error" << std::endl;
  }

  onCreate();
  FPS::startFPS();
  while (!shouldQuit()) {
    //openVR update
    openVR->update();

    onAnimate(dt_sec()); // millis for dt

    //openVR draw
    openVR->draw(std::bind(&VRApp::onDraw, this, std::placeholders::_1), mGraphics);

    // Draw into window
    //g.pushViewport(openVR->fbo.width(), openVR->fbo.height());
    auto view = openVR->eyePosLeft * openVR->HMDPose;
    auto proj = openVR->projectionLeft;
    mGraphics.pushProjMatrix(proj);
    mGraphics.pushViewMatrix(view);
    onDraw(mGraphics);
    mGraphics.popProjMatrix();
    mGraphics.popViewMatrix();
    //g.popViewport();

    Window::refresh();
    FPS::tickFPS();
  }
  onExit();
  openVR->close();
  Window::destroy();
  glfw::terminate(is_verbose);
}


} // namespace al

#endif
