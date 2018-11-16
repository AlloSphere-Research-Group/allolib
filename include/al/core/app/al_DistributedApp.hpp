#ifndef INCLUDE_AL_DISTRIBUTEDAPP_HPP
#define INCLUDE_AL_DISTRIBUTEDAPP_HPP

/*  Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/app/al_WindowApp.hpp"
#include "al/core/app/al_AudioApp.hpp"
#include "al/util/al_FlowAppParameters.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/core/protocol/al_OSC.hpp"
#include "al/core/graphics/al_GLFW.hpp"
#include "al/core/io/al_Window.hpp"
#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/io/al_ControlNav.hpp"
#include "al/sphere/al_OmniRenderer.hpp"
#include "al/util/al_Toml.hpp"

#include <iostream>
#include <map>

#ifdef AL_BUILD_MPI
#include <mpi.h>
#include <unistd.h>
#endif

#ifdef AL_USE_CUTTLEBONE
#include "Cuttlebone/Cuttlebone.hpp"
#endif

/*  Keehong Youn, 2017, younkeehong@gmail.com
 *  Andres Cabrera, 2018, mantaraya36@gmail.com
 *
 * Using this file requires Cuttlebone. MPI is optional.
*/

namespace al {

struct BasicState {
	Pose pose;
};

template<class TSharedState = BasicState>
class DistributedApp: public OmniRenderer,
           public AudioApp,
           public FlowAppParameters,
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
      ROLE_DESKTOP, // Application runs as single desktop app
      ROLE_USER // User defined roles can add from here (using bitshift to use the higher bits)
  } Role;

  DistributedApp() {

#ifdef AL_BUILD_MPI
      MPI_Init(NULL, NULL);

      MPI_Comm_size(MPI_COMM_WORLD, &world_size);
      MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

      MPI_Get_processor_name(processor_name, &name_len);
#endif
      if (isMaster()) {
          configLoader.setFile("distributed_app.toml");
          configLoader.setDefaultValue("distributed", false);
          configLoader.setDefaultValue("broadcastAddress", std::string("192.168.0.255"));
          configLoader.writeFile();
          if (configLoader.getb("distributed")) {
              std::cout << name() << ":Running distributed" << std::endl;
              mRole = ROLE_SIMULATOR;
              mRunDistributed = true;
          }
      }

      if (mRunDistributed) {

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

//          if (isMaster()) {
//              int number = 398;
//              for (int i = 1; i < world_size; i ++) {
//                  MPI_Send(&number, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
//              }
//          }
          for (auto entry: mRoleMap) {
              if (strncmp(name().c_str(), entry.first.c_str(), name().size()) == 0) {
				  if (entry.second.size() == 1) {
					  mRole = entry.second[0];
				  } else {
#ifdef AL_BUILD_MPI
					  int rank = world_rank;
#else
					  int rank = 0;
#endif
                      if (rank < (int) entry.second.size()) {
                          mRole = entry.second[rank];
                      } else {
                          mRole = entry.second[0];
                      }
                  }
              }
          }
      }
#ifdef AL_BUILD_MPI
      if (!isMaster()) {
//          int number;
//          MPI_Recv(&number, 1, MPI_INT, 0, 1, MPI_COMM_WORLD,
//                   MPI_STATUS_IGNORE);
          mRole = ROLE_RENDERER;
      }
      std::cout << name() << ":" << world_rank << " pid: "<< getpid()<< " set role to " << roleName() << std::endl;
#endif
  }

  ~DistributedApp() {
#ifdef AL_USE_CUTTLEBONE
      if (mMaker) {
          mMaker->stop();
      } else if (mTaker){
          mTaker->stop();
      }
#endif
#ifdef AL_BUILD_MPI
      MPI_Finalize();
#endif
  }

  std::string name() {
#ifdef AL_BUILD_MPI
      return processor_name;
#else
      return "localhost";
#endif
  }

  Role role() {return mRole;}

  int rank() {
#ifdef AL_BUILD_MPI
      return world_rank;
#else
      return 0;
#endif
  }

  std::string roleName() {
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
      case ROLE_DESKTOP:
          return "desktop";
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
  Nav& nav() override { return mNav; }
  const Nav& nav() const { return mNav; }

  Pose& pose() override { return mNav; }
  const Pose& pose() const override { return mNav; }

  NavInputControl& navControl() { return mNavControl; }
  NavInputControl const& navControl() const { return mNavControl; }

  Lens& lens() override { return mView.lens(); }
  Lens const& lens() const override { return mView.lens(); }

  // overrides (WindowApp & Omnirenderer)'s start to also initiate AudioApp and etc.
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

  void syncrhonize() {
#ifdef AL_BUILD_MPI
      MPI_Barrier(MPI_COMM_WORLD); // Wait for everybody
#endif
  }

  void log(std::string logText) {
      std::cout << name() << ":" << logText << std::endl;
  }

  ParameterServer &parameterServer() override { return *mParameterServer; }

private:

#ifdef AL_BUILD_MPI
  // MPI data
  int world_size;
  int world_rank;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
#endif
  bool mRunDistributed {false};
  Role mRole {ROLE_DESKTOP};

  std::map<std::string, std::vector<Role>> mRoleMap;

  TSharedState mState;
  int mQueuedStates {0};
#ifdef AL_USE_CUTTLEBONE
  std::unique_ptr<cuttlebone::Maker<TSharedState>> mMaker;
  std::unique_ptr<cuttlebone::Taker<TSharedState>> mTaker;
#endif
  std::shared_ptr<ParameterServer> mParameterServer;

  TomlLoader configLoader;

};


// ---------- IMPLEMENTATION ---------------------------------------------------

template<class TSharedState>
inline void DistributedApp<TSharedState>::start() {
  glfw::init(is_verbose);

  uint16_t receiverPort = 9100;
  if (role() == ROLE_SIMULATOR || role() == ROLE_DESKTOP) {
      mParameterServer = std::make_shared<ParameterServer>("", 9010);
      for (auto member: mRoleMap) {
          // Relay all parameters to renderers
          if (std::find(member.second.begin(), member.second.end(), ROLE_RENDERER) != member.second.end()) {
              mParameterServer->addListener(member.first, receiverPort);
              continue;
          }
      }
  } else {
      mParameterServer = std::make_shared<ParameterServer>("", receiverPort);
  }
//  std::cout << name() << ":" << roleName()  << " before onInit" << std::endl;
  onInit();

  // must do before Window::create, overrides user given window diemnsions
  check_if_in_sphere_and_setup_window_dimensions();

  Window::create(is_verbose);
  preOnCreate();
//  std::cout << name() << ":" << roleName()  << " before onCreate" << std::endl;
  onCreate();

  AudioApp::beginAudio(); // only begins if `initAudio` was called before
  // initDeviceServer();

//  std::cout << name() << ":" << roleName() << " before init flow" << std::endl;
  if (role() == ROLE_SIMULATOR || role() == ROLE_DESKTOP) initFlowApp(true);
  else initFlowApp(false);
  
  mParameterServer->registerOSCListener(this); // Have the parameter server pass unhandled messages to this app's onMessage virtual function

  FPS::startFPS(); // WindowApp (FPS)

  while (!WindowApp::shouldQuit()) {
    // to quit, call WindowApp::quit() or click close button of window,
    // or press ctrl + q
    if (role() == ROLE_DESKTOP || role() == ROLE_SIMULATOR) {
      simulate(dt_sec());
      mQueuedStates = 1;
#ifdef AL_USE_CUTTLEBONE
      if (mMaker) {
        mMaker->set(mState);
      }
#endif
    } else {
#ifdef AL_USE_CUTTLEBONE
        if (mTaker) {
            mQueuedStates = mTaker->get(mState);
        }
#else
        // You shouldn't get here if you are relying oncuttlebone for state syncing
        mQueuedStates = 1;
#endif
    }

    preOnAnimate(dt_sec());
    onAnimate(dt_sec());
    if (role() == ROLE_RENDERER && running_in_sphere_renderer) {
      draw_using_perprojection_capture();
    }
    else {
      preOnDraw();
      onDraw(mGraphics);
      postOnDraw();
    }
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
#ifdef AL_USE_CUTTLEBONE
  if (role() == ROLE_SIMULATOR) {
      std::string broadcastAddress = configLoader.gets("broadcastAddress");
      mMaker = std::make_unique<cuttlebone::Maker<TSharedState>>(broadcastAddress.c_str());
      mMaker->start();
  } else if (role() == ROLE_RENDERER){
      mTaker = std::make_unique<cuttlebone::Taker<TSharedState>>();
      mTaker->start();
  }
#endif

  window_is_stereo_buffered = Window::displayMode() & Window::STEREO_BUF;
  mGraphics.init();
  if (role() == ROLE_RENDERER && running_in_sphere_renderer) {
    load_perprojection_configuration();
  }
  if (role() == ROLE_RENDERER) {
    cursorHide(true);
  }
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
