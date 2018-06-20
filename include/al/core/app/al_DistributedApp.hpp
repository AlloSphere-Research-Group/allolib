#ifndef INCLUDE_AL_DISTRIBUTEDAPP_HPP
#define INCLUDE_AL_DISTRIBUTEDAPP_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/app/al_WindowApp.hpp"
#include "al/core/app/al_AudioApp.hpp"
#include "al/util/al_DeviceServerApp.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/core/protocol/al_OSC.hpp"
#include "al/core/graphics/al_GLFW.hpp"
#include "al/core/io/al_Window.hpp"
#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/io/al_ControlNav.hpp"
#include "al/sphere/al_OmniRenderer.hpp"

#include <iostream>
#include <map>

#ifdef AL_BUILD_MPI
#include <mpi.h>
#endif

#include "Cuttlebone/Cuttlebone.hpp"

/*  Keehong Youn, 2017, younkeehong@gmail.com
 *  Andres Cabrera, 2018, mantaraya36@gmail.com
 *
 * Using this file requires both MPI and Cuttlebone
*/

namespace al {

template<class TSharedState>
class DistributedApp: public OmniRenderer,
           public AudioApp,
           // public DeviceServerApp,
           public osc::PacketHandler
{
  Nav mNav; // is a Pose itself and also handles manipulation of pose
  Viewpoint mView {mNav.transformed()};  // Pose with Lens and acts as camera
  NavInputControl mNavControl {mNav}; // interaction with keyboard and mouse

public:

  typedef enum {
      ROLE_NONE = 0,
      ROLE_SIMULATOR,
      ROLE_RENDERER,
      ROLE_AUDIO,
      ROLE_CONTROL,
      ROLE_INTERFACE, // For interface server
      ROLE_USER // User defined roles can add from here (using bitshift to use the higher bits)
  } Role;

  DistributedApp() {
#ifdef AL_BUILD_MPI
      MPI_Init(NULL, NULL);

      MPI_Comm_size(MPI_COMM_WORLD, &world_size);
      MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

      MPI_Get_processor_name(processor_name, &name_len);

      mRoleMap["spherez05"] = {ROLE_SIMULATOR, ROLE_RENDERER};
      mRoleMap["gr01"] = {ROLE_SIMULATOR};

      mRoleMap["moxi"] = {ROLE_RENDERER};
      mRoleMap["gr02"] = {ROLE_RENDERER};
      mRoleMap["gr03"] = {ROLE_RENDERER};
      mRoleMap["gr04"] = {ROLE_RENDERER};
      mRoleMap["gr05"] = {ROLE_RENDERER};
      mRoleMap["gr06"] = {ROLE_RENDERER};
      mRoleMap["gr07"] = {ROLE_RENDERER};
      mRoleMap["gr08"] = {ROLE_RENDERER};
      mRoleMap["gr09"] = {ROLE_RENDERER};
      mRoleMap["gr10"] = {ROLE_RENDERER};
      mRoleMap["gr11"] = {ROLE_RENDERER};
      mRoleMap["gr12"] = {ROLE_RENDERER};
      mRoleMap["gr13"] = {ROLE_RENDERER};
      mRoleMap["gr14"] = {ROLE_RENDERER};

      mRoleMap["audio"] = {ROLE_AUDIO};
      mRoleMap["ar01"] = {ROLE_AUDIO};

      for (auto entry: mRoleMap) {
          if (strncmp(processor_name, entry.first.c_str(), name_len) == 0) {
              if (entry.second.size() == 1) {
                  mRole = entry.second[0];
              } else {
                  if (world_rank < entry.second.size()) {
                      mRole = entry.second[world_rank];
                  } else {
                      mRole = entry.second[0];
                  }
              }
          }
      }
      std::cout << name() << ":" << world_rank << " set role to " << roleName() << std::endl;
#else
      mRole = ROLE_SIMULATOR;
#endif

  }

  ~DistributedApp() {
      if (role() == ROLE_SIMULATOR) {
          mMaker.stop();
      } else {
          mTaker.stop();
      }
#ifdef AL_BUILD_MPI
      MPI_Finalize();
#endif
  }

  const char *name() {
#ifdef AL_BUILD_MPI
      return processor_name;
#else
      return "localhost";
#endif
  }

  Role role() {return mRole;}

  string roleName() {
      switch (mRole) {
      case ROLE_SIMULATOR:
          return "simulator";
      case ROLE_AUDIO:
          return "audio";
      case ROLE_RENDERER:
          return "renderer";
      case ROLE_CONTROL:
          return "control";
      case ROLE_INTERFACE:
          return "interface";
      case ROLE_NONE:
          return "none";
      default:
          return "[user role]";
      }
  }

  void print() {
#ifdef AL_BUILD_MPI
      std::cout << "Processor: " << processor_name
                << " Rank: " << world_rank
                << " Of: " << world_size << std::endl;
      std::cout << name() << ":" << world_rank << " set role to " << roleName() << std::endl;
#else
      std::cout << "DistributedApp: Not using MPI. Role: " << roleName() << std::endl;
#endif
  }

  bool isMaster() {
#ifdef AL_BUILD_MPI
      return world_rank == 0;
#else
      return true;
#endif
  }


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

  /// Override to compute updates to shared state
  /// Currently run before calls to onAnimate()
  /// i.e. this is synchronous to drawing and runs at
  /// frame rate
  virtual void simulate(double dt) {}

  // extra functionalities to be handled
  virtual void preOnCreate();
  virtual void preOnAnimate(double dt);
  virtual void preOnDraw();
  virtual void postOnDraw();
  virtual void postOnExit();

  // PacketHandler
  void onMessage(osc::Message& m) override {}

  /**
   * @brief get current shared state
   * @return reference to shared state.
   *
   * State can be modified if role() == ROLE_SIMULATOR
   * Otherwise any changes made to state will not propagate.
   */
  TSharedState &state() { return mState;}
  /**
   * @brief returns the number of states received
   *
   * The number of states is updated prior to onAnimate() and onDraw()
   * so it only really makes sense to check this within these two
   * functions.
   */
  int newStates() { return mQueuedStates; }

  void log(std::string logText) {
      std::cout << name() << ":" << logText << std::endl;
  }

  ParameterServer &parameterServer() { return *mParameterServer; }

private:

#ifdef AL_BUILD_MPI
  // MPI data
  int world_size;
  int world_rank;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
#endif
  Role mRole {ROLE_NONE};

  std::map<string, std::vector<Role>> mRoleMap;

  TSharedState mState;
  int mQueuedStates {0};
  cuttlebone::Maker<TSharedState> mMaker {"192.168.0.255"};
  cuttlebone::Taker<TSharedState> mTaker;
  std::shared_ptr<ParameterServer> mParameterServer;

};


// ---------- IMPLEMENTATION ---------------------------------------------------

template<class TSharedState>
inline void DistributedApp<TSharedState>::start() {
  glfw::init(is_verbose);

  uint16_t receiverPort = 9100;
  if (role() == ROLE_SIMULATOR) {
      mParameterServer = make_shared<ParameterServer>("", 9010);
      for (auto member: mRoleMap) {
          if (std::find(member.second.begin(), member.second.end(), ROLE_RENDERER) != member.second.end()) {
              mParameterServer->addListener(member.first, receiverPort);
              continue;
          }
      }
  } else {
      mParameterServer = make_shared<ParameterServer>("", receiverPort);
  }
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

    if (role() == ROLE_SIMULATOR) {
      simulate(dt_sec());
      mMaker.set(mState);
      mQueuedStates = 1;
    } else {
      mQueuedStates = mTaker.get(mState);
    }

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

template<class TSharedState>
inline void DistributedApp<TSharedState>::preOnCreate() {
  append(mNavControl);

  if (role() == ROLE_SIMULATOR) {
      mMaker.start();
  } else {
      mTaker.start();
  }
  mGraphics.init();
}

template<class TSharedState>
inline void DistributedApp<TSharedState>::preOnAnimate(double dt) {
    mNav.smooth(std::pow(0.0001, dt));
    mNav.step(dt * fps());
}

template<class TSharedState>
inline void DistributedApp<TSharedState>::preOnDraw() {
    mGraphics.framebuffer(FBO::DEFAULT);
    mGraphics.viewport(0, 0, fbWidth(), fbHeight());
    mGraphics.resetMatrixStack();
    mGraphics.camera(mView);
    mGraphics.color(1, 1, 1);
}

template<class TSharedState>
inline void DistributedApp<TSharedState>::postOnDraw() {
  //
}

template<class TSharedState>
inline void DistributedApp<TSharedState>::postOnExit() {
  //
}

}


#endif
