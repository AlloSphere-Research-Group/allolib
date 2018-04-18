#ifndef INCLUDE_AL_DISTRIBUTEDAPP_HPP
#define INCLUDE_AL_DISTRIBUTEDAPP_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/app/al_WindowApp.hpp"
#include "al/core/app/al_AudioApp.hpp"
#include "al/util/al_DeviceServerApp.hpp"
#include "al/core/protocol/al_OSC.hpp"
#include "al/core/graphics/al_GLFW.hpp"
#include "al/core/io/al_Window.hpp"
#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/io/al_ControlNav.hpp"

#include <iostream>

#ifdef AL_BUILD_MPI
#include <mpi.h>
#endif

/*  Keehong Youn, 2017, younkeehong@gmail.com
 *  Andres Cabrera, 2018, mantaraya36@gmail.com
*/

namespace al {

class DistributedApp: public WindowApp,
           public AudioApp,
           // public DeviceServerApp,
           public osc::PacketHandler
{
  Nav mNav; // is a Pose itself and also handles manipulation of pose
  Viewpoint mView {mNav.transformed()};  // Pose with Lens and acts as camera
  NavInputControl mNavControl {mNav}; // interaction with keyboard and mouse

public:

  typedef enum {
      ROLE_SIMULATOR,
      ROLE_RENDERER,
      ROLE_AUDIO,
      ROLE_CONTROL
  } Role;

  DistributedApp() {
      MPI_Init(NULL, NULL);

      MPI_Comm_size(MPI_COMM_WORLD, &world_size);
      MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

      MPI_Get_processor_name(processor_name, &name_len);
  }

  ~DistributedApp() {
      MPI_Finalize();
  }

  void print() {
      std::cout << "Processor: " << processor_name
                << " Rank: " << world_rank
                << " Of: " << world_size << std::endl;
  }

  bool isMaster() {return world_rank == 0;}

  virtual void simulate() {};


  Viewpoint& view() { return mView; }
  const Viewpoint& view() const { return mView; }

  // Nav& nav() override { return mNav; }
  Nav& nav() { return mNav; }
  const Nav& nav() const { return mNav; }

  Pose& pose() { return mNav; }
  const Pose& pose() const { return mNav; }

  NavInputControl& navControl() { return mNavControl; }
  NavInputControl const& navControl() const { return mNavControl; }

  Lens& lens() { return mView.lens(); }
  Lens const& lens() const { return mView.lens(); }

  Graphics& graphics() { return mGraphics; }

  // overrides WindowApp's start to also initiate AudioApp and etc.
  void start() override;

  // interface from WindowApp
  // users override these
  void onInit() override {}
  void onCreate() override {}
  void onAnimate(double dt) override {}
  void onDraw (Graphics& g) override {}
  void onExit() override {}
  void onKeyDown(Keyboard const& k) override {}
  void onKeyUp(Keyboard const& k) override {}
  void onMouseDown(Mouse const& m) override {}
  void onMouseUp(Mouse const& m) override {}
  void onMouseDrag(Mouse const& m) override {}
  void onMouseMove(Mouse const& m) override {}
  void onResize(int w, int h) override {}
  void onVisibility(bool v) override {}

  // extra functionalities to be handled
  virtual void preOnCreate();
  virtual void preOnAnimate(double dt);
  virtual void preOnDraw();
  virtual void postOnDraw();
  virtual void postOnExit();

  // PacketHandler
  void onMessage(osc::Message& m) override {}

private:

  // MPI data
  int world_size;
  int world_rank;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  Role mRole {ROLE_SIMULATOR};
};


// ---------- IMPLEMENTATION ---------------------------------------------------

inline void DistributedApp::start() {
  glfw::init(is_verbose);
  onInit();
  Window::create(is_verbose);
  preOnCreate();
  onCreate();
  AudioApp::beginAudio(); // only begins if `initAudio` was called before
  FPS::startFPS(); // WindowApp (FPS)
  // initDeviceServer();

  while (!WindowApp::shouldQuit()) {
    // to quit, call WindowApp::quit() or click close button of window,
    // or press ctrl + q
    preOnAnimate(dt_sec());
    onAnimate(dt_sec());
    preOnDraw();
    onDraw(mGraphics);
    postOnDraw();
    Window::refresh();
    FPS::tickFPS();
  }

  onExit(); // user defined
  postOnExit();
  AudioApp::endAudio(); // AudioApp
  Window::destroy();
  glfw::terminate(is_verbose);

}

inline void DistributedApp::preOnCreate() {
  append(mNavControl);
  mGraphics.init();
}

inline void DistributedApp::preOnAnimate(double dt) {
    mNav.smooth(std::pow(0.0001, dt));
    mNav.step(dt * fps());
}

inline void DistributedApp::preOnDraw() {
    mGraphics.framebuffer(FBO::DEFAULT);
    mGraphics.viewport(0, 0, fbWidth(), fbHeight());
    mGraphics.resetMatrixStack();
    mGraphics.camera(mView);
    mGraphics.color(1, 1, 1);
}

inline void DistributedApp::postOnDraw() {
  //
}

inline void DistributedApp::postOnExit() {
  //
}

}


#endif
