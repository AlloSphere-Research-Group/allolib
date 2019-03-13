#ifndef INCLUDE_AL_DISTRIBUTEDAPP_HPP
#define INCLUDE_AL_DISTRIBUTEDAPP_HPP

/* Keehong Youn, 2017, younkeehong@gmail.com
 * Andres Cabrera, 2018, 2019, mantaraya36@gmail.com
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
#include "al/util/al_Socket.hpp"
#include "al/util/scene/al_DynamicScene.hpp"
#include "al/util/scene/al_DistributedScene.hpp"

#include <iostream>
#include <map>

#ifdef AL_BUILD_MPI
#include <mpi.h>
#include <unistd.h>
#endif

#ifdef AL_USE_CUTTLEBONE
#include "Cuttlebone/Cuttlebone.hpp"
#endif

/*
 * MPI and cuttlebone are optional.
*/

namespace al {

struct DefaultState {
	Pose pose;
};

template<class TSharedState = DefaultState>
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
      ROLE_SIMULATOR = 1 << 1,
      ROLE_RENDERER = 1 << 2,
      ROLE_AUDIO = 1 << 3,
      ROLE_CONTROL = 1 << 4,
      ROLE_INTERFACE = 1 << 5, // For interface server
      ROLE_DESKTOP = 1 << 6, // Application runs as single desktop app
      ROLE_DESKTOP_REPLICA = 1 << 7, // Application runs as single desktop app
      ROLE_USER  = 1 << 8 // User defined roles can add from here through bitshifting
  } Role;

  DistributedApp(bool runDistributed = DistributedApp::shouldRunDistributed()) {

#ifdef AL_BUILD_MPI
      MPI_Init(NULL, NULL);

      MPI_Comm_size(MPI_COMM_WORLD, &world_size);
      MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

      MPI_Get_processor_name(processor_name, &name_len);
#endif
      if (runDistributed) {
        mRunDistributed = true;
      }

      if (mRunDistributed) {

          mRoleMap["spherez05"] = ROLE_SIMULATOR;
          mRoleMap["gr01"] = ROLE_SIMULATOR;

          mRoleMap["moxi"] = ROLE_RENDERER;
          mRoleMap["gr02"] = ROLE_RENDERER;
          mRoleMap["gr03"] = ROLE_RENDERER;
          mRoleMap["gr04"] = ROLE_RENDERER;
          mRoleMap["gr05"] = ROLE_RENDERER;
          mRoleMap["gr06"] = ROLE_RENDERER;
          mRoleMap["gr07"] = ROLE_RENDERER;
          mRoleMap["gr08"] = ROLE_RENDERER;
          mRoleMap["gr09"] = ROLE_RENDERER;
          mRoleMap["gr10"] = ROLE_RENDERER;
          mRoleMap["gr11"] = ROLE_RENDERER;
          mRoleMap["gr12"] = ROLE_RENDERER;
          mRoleMap["gr13"] = ROLE_RENDERER;
          mRoleMap["gr14"] = ROLE_RENDERER;

          mRoleMap["audio"] = (Role) (ROLE_AUDIO | ROLE_SIMULATOR);
          mRoleMap["audio.10g"] = (Role) (ROLE_AUDIO | ROLE_SIMULATOR);
          mRoleMap["ar01"] = ROLE_AUDIO;

//          if (isMaster()) {
//              int number = 398;
//              for (int i = 1; i < world_size; i ++) {
//                  MPI_Send(&number, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
//              }
//          }

          mRole = ROLE_NONE;
          for (auto entry: mRoleMap) {
              if (strncmp(name().c_str(), entry.first.c_str(), name().size()) == 0) {
                mRole = entry.second;
                std::cout << name() << ":Running distributed as " << roleName() << std::endl;
              }
          }
          if (hasRole(ROLE_SIMULATOR)) {
              configLoader.setFile("distributed_app.toml");
              configLoader.setDefaultValue("broadcastAddress", std::string("192.168.0.255"));
              configLoader.writeFile();
              std::cout << "Primary: " << name() << ":Running distributed" << std::endl;
          }
      }


#ifdef AL_BUILD_MPI
      if (!isPrimary()) {
//          int number;
//          MPI_Recv(&number, 1, MPI_INT, 0, 1, MPI_COMM_WORLD,
//                   MPI_STATUS_IGNORE);
        if(getenv("OMPI_COMM_WORLD_RANK")) {
              mRole = ROLE_RENDERER;
        }
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
    if (mRunDistributed) {
      return al_get_hostname();
    } else {
      return "localhost";
    }
#endif
  }

  Role role() { return mRole;}

  bool hasRole(Role role) { return mRole & role;}

  int rank() {
#ifdef AL_BUILD_MPI
      if(getenv("OMPI_COMM_WORLD_RANK")) {
          return world_rank;
      } else {
          return 0;
      }
#else
      return 0;
#endif
  }

  std::string roleName() {
    std::string name;
    if (mRole & ROLE_SIMULATOR) {
      name += "simulator+";
    }
    if (mRole & ROLE_RENDERER) {
      name += "renderer+";
    }
    if (mRole & ROLE_AUDIO) {
      name += "audio+";
    }
    if (mRole & ROLE_CONTROL) {
      name += "control+";
    }
    if (mRole & ROLE_INTERFACE) {
      name += "interface+";
    }
    if (mRole & ROLE_DESKTOP) {
      name += "desktop+";
    }
    if (mRole & ROLE_DESKTOP_REPLICA) {
      name += "replica+";
    }
    if (name.size() == 0 && mRole != ROLE_NONE) {
      // TODO show user role index (amount of bitshift from ROLE_USER)
      name += "[user role]+";
    }
    if (name.size() > 0) {
      name = name.substr(0, name.size() -1);
    } else {
      name = "none";
    }
    return name;
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

  bool isPrimary() {
#ifdef AL_BUILD_MPI
      if(getenv("OMPI_COMM_WORLD_RANK")) {
          return world_rank == 0;
      } else {
          return hasRole(ROLE_SIMULATOR) || hasRole(ROLE_DESKTOP);
      }
#else
      return hasRole(ROLE_SIMULATOR) || hasRole(ROLE_DESKTOP);
#endif
  }

  virtual void initAudio (double audioRate, int audioBlockSize,
                          int audioOutputs, int audioInputs,
                          int device = -1) override {
    if (hasRole(ROLE_AUDIO) || hasRole(ROLE_DESKTOP) ||  hasRole(ROLE_DESKTOP_REPLICA)) {
      AudioApp::initAudio(audioRate, audioBlockSize, audioOutputs, audioInputs, device);
    } else {
      std::cout << "Audio disabled on this node.";
    }
  }

  virtual void initAudio (AudioIOConfig config = OUT_ONLY) override {
    if (hasRole(ROLE_AUDIO) || hasRole(ROLE_DESKTOP) ||  hasRole(ROLE_DESKTOP_REPLICA)) {
      AudioApp::initAudio(config);
    } else {
      std::cout << "Audio disabled on this node.";
    }
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
   * State should only be modified if hasRole(ROLE_SIMULATOR)
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

  static bool shouldRunDistributed() {
    std::vector<std::string> distributedNodes =
    { "gr01", "gr02", "gr03", "gr04", "gr05", "gr06", "gr07", "gr08", "gr09",
      "gr10", "gr11", "gr12", "gr13", "gr14",
      "audio", "audio.10g", "ar01", "spherez05", "interface", "control"};
    for (auto nodeName: distributedNodes) {
      if (al_get_hostname() == nodeName) {
        return true;
      }
    }
    return false;
  }

  void registerDynamicScene(DynamicScene &scene) {

      if (isPrimary()) {
          if (dynamic_cast<DistributedScene *>(&scene)) {
              dynamic_cast<DistributedScene *>(&scene)->registerNotifier(parameterServer());

          }
      } else {
          parameterServer().registerOSCConsumer(
                      dynamic_cast<DistributedScene *>(&scene), "");
      }
      scene.prepare(audioIO());
  }
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

  std::map<std::string, Role> mRoleMap;

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

  uint16_t sendPort = 9010;
  uint16_t receiverPort = 9100;
  std::string defaultAddress = "0.0.0.0";
  if (role() & ROLE_SIMULATOR || role() & ROLE_DESKTOP) {
    mParameterServer = std::make_shared<ParameterServer>(defaultAddress, sendPort);
    if (mParameterServer->serverRunning()) {
      //          mParameterServer->addListener("127.0.0.1", 9100);
      mParameterServer->startHandshakeServer(defaultAddress);
      for (auto member: mRoleMap) {
        // Relay all parameters to renderers
        if (member.second & ROLE_RENDERER) {
          std::cout << "Added renderer as listener " << member.first << ":" << receiverPort << std::endl;
          mParameterServer->addListener(member.first, receiverPort);
          continue;
        }
      }
    } else {
      int portOffset = 0;
      int maxInstances = 64;
      while (!mParameterServer->serverRunning() && portOffset < maxInstances) {
        mParameterServer->listen(receiverPort + portOffset++, defaultAddress);
      }
      if (mParameterServer->serverRunning()) {

        mParameterServer->startCommandListener(defaultAddress);
        mRole = ROLE_DESKTOP_REPLICA;
        std::cout << "Application is replica on port: " << mParameterServer->serverPort() << std::endl;
      } else {
        std::cerr << "Warning: Application has no network role." << std::endl;
        mRole = ROLE_NONE;
      }
    }
  } else {
    mParameterServer = std::make_shared<ParameterServer>(defaultAddress, receiverPort);
  }
  //  std::cout << name() << ":" << roleName()  << " before onInit" << std::endl;
  onInit();

  // must do before Window::create, overrides user given window diemnsions
  check_if_in_sphere_and_setup_window_dimensions();

  Window::create(is_verbose);
  preOnCreate();
  //  std::cout << name() << ":" << roleName()  << " before onCreate" << std::endl;
  onCreate();

  if(hasRole(ROLE_AUDIO) || hasRole(ROLE_DESKTOP) || hasRole(ROLE_DESKTOP_REPLICA)) {
    AudioApp::beginAudio(); // only begins if `initAudio` was called before
  }

  //  std::cout << name() << ":" << roleName() << " before init flow" << std::endl;
  if (role() & ROLE_SIMULATOR || role() & ROLE_DESKTOP) initFlowApp(true);
  else initFlowApp(false);
  
  if (mParameterServer) {
    mParameterServer->registerOSCListener(this); // Have the parameter server pass unhandled messages to this app's onMessage virtual function
    std::cout << "Registered parameter server with Distributed App network socket" <<std::endl;
  }

  FPS::startFPS(); // WindowApp (FPS)

  while (!WindowApp::shouldQuit()) {
    // to quit, call WindowApp::quit() or click close button of window,
    // or press ctrl + q
    if (role() & ROLE_DESKTOP || role() & ROLE_SIMULATOR) {
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
    bool forceOmni = false;
    bool drawOmni = (hasRole(ROLE_RENDERER) && running_in_sphere_renderer) || forceOmni;
    if (drawOmni) {
      draw_using_perprojection_capture();
    }
    else { // Not Omni
      if (render_stereo) {
        preOnDraw();
        // check stereo window and do below to render in stereo when not in sphere
        glDrawBuffer(GL_BACK_LEFT);
        mGraphics.eye(Graphics::LEFT_EYE);
        onDraw(mGraphics);
        postOnDraw();

        preOnDraw();
        glDrawBuffer(GL_BACK_RIGHT);
        mGraphics.eye(Graphics::RIGHT_EYE);
        onDraw(mGraphics);
        postOnDraw();
        glDrawBuffer(GL_BACK_LEFT);
        mGraphics.eye(Graphics::MONO_EYE);
        
      } else {
        preOnDraw();
        onDraw(mGraphics);
        postOnDraw();
      }
    }
    Window::refresh();
    FPS::tickFPS();
  }

  onExit(); // user defined
  postOnExit();
  if(hasRole(ROLE_AUDIO) || hasRole(ROLE_DESKTOP) || hasRole(ROLE_DESKTOP_REPLICA)) {
    AudioApp::endAudio(); // AudioApp
  }
  Window::destroy();
  glfw::terminate(is_verbose);

}

template<class TSharedState>
inline void DistributedApp<TSharedState>::preOnCreate() {
  append(mNavControl);
#ifdef AL_USE_CUTTLEBONE
  if (role() & ROLE_SIMULATOR) {
      std::string broadcastAddress = configLoader.gets("broadcastAddress");
      mMaker = std::make_unique<cuttlebone::Maker<TSharedState>>(broadcastAddress.c_str());
      mMaker->start();
  } else if (role() & ROLE_RENDERER){
      mTaker = std::make_unique<cuttlebone::Taker<TSharedState>>();
      mTaker->start();
  }
#endif

  window_is_stereo_buffered = Window::displayMode() & Window::STEREO_BUF;
  mGraphics.init();
  if (role() & ROLE_RENDERER && running_in_sphere_renderer) {
    load_perprojection_configuration();
  }
  if (role() & ROLE_RENDERER) {
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
