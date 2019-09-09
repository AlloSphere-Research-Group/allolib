#include "al/app/al_DistributedApp.hpp"

using namespace al;

DistributedApp::DistributedApp() : App() {}

void DistributedApp::initialize() {
#ifdef AL_WINDOWS
  // Required to make sure gethostname() works....
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;
  wVersionRequested = MAKEWORD(2, 2);
  err = WSAStartup(wVersionRequested, &wsaData);
  if (err != 0) {
    std::cerr << "WSAStartup failed with error: " << err << std::endl;
  }
#endif
  TomlLoader appConfig("distributed_app.toml");
  auto nodesTable = appConfig.root->get_table_array("node");
  // First get role from config file
  if (nodesTable) {
    for (const auto &table : *nodesTable) {
      std::string host = *table->get_as<std::string>("host");
      std::string role = *table->get_as<std::string>("role");

      if (strncmp(name().c_str(), host.c_str(), name().size()) == 0) {
        // Now set capabilities from role
        if (role == "desktop") {
          mCapabilites = (Capability)(CAP_SIMULATOR | CAP_RENDERING |
                                      CAP_AUDIO_IO | CAP_OSC);
        } else if (role == "renderer") {
          mCapabilites =
              (Capability)(CAP_SIMULATOR | CAP_OMNIRENDERING | CAP_OSC);
        } else if (role == "audio") {
          mCapabilites = (Capability)(CAP_SIMULATOR | CAP_AUDIO_IO |
                                      CAP_CONSOLE_IO | CAP_OSC);
        } else if (role == "simulator") {
          mCapabilites = (Capability)(CAP_SIMULATOR | CAP_CONSOLE_IO | CAP_OSC);
        } else if (role == "replica") {
          mCapabilites = (Capability)(CAP_SIMULATOR | CAP_OMNIRENDERING |
                                      CAP_AUDIO_IO | CAP_OSC);
        } else if (role == "control") {
          mCapabilites = (Capability)(CAP_RENDERING | CAP_OSC);
        } else {
          std::cerr << "WARNING: Setting no capabilities for this app from "
                       "config file"
                    << std::endl;
        }
      }
      mRoleMap[host] = role;
      if (table->contains("dataRoot") &&
          strncmp(name().c_str(), host.c_str(), name().size()) ==
              0) { // Set configuration for this node when found
        std::string dataRootValue = *table->get_as<std::string>("dataRoot");
        mGlobalDataRootPath = File::conformPathToOS(dataRootValue);
      } else {
        std::cout << "WARNING: node " << host.c_str() << " not given dataRoot"
                  << std::endl;
      }

      if (table->contains("rank")) {
        rank = *table->get_as<int>("rank");
      } else {
        std::cout << "WARNING: node " << host.c_str() << " not given rank"
                  << std::endl;
      }
      if (table->contains("group")) {
        group = *table->get_as<int>("group");
      } else {
        std::cout << "WARNING: node " << host.c_str() << " not given group"
                  << std::endl;
      }
    }
  } else { // No nodes table in config file. Use desktop role
    mCapabilites =
        (Capability)(CAP_SIMULATOR | CAP_RENDERING | CAP_AUDIO_IO | CAP_OSC);
    group = 0;
  }

  //      if (mRunDistributed) {
  //          for (auto entry: mRoleMap) {
  //              if (strncmp(name().c_str(), entry.first.c_str(),
  //              name().size()) == 0) {
  //                mRole = entry.second;
  ////                std::cout << name() << ":Running distributed as " <<
  /// roleName() << std::endl;
  //              }
  //          }

  //      }
  if (hasCapability(CAP_SIMULATOR)) {

    TomlLoader configLoader;
    configLoader.setFile("distributed_app.toml");
    configLoader.setDefaultValue("broadcastAddress",
                                 std::string("192.168.0.255"));
    configLoader.writeFile();
  }

  osc::Recv testServer;
  // probe to check if first port available, this will determine if this
  // application is the primary or the replica
  if (!testServer.open(mOSCDomain->port, mOSCDomain->interfaceIP.c_str())) {
    mCapabilites = (Capability)(CAP_SIMULATOR | CAP_OMNIRENDERING | CAP_OSC);
    rank = 1;
    std::cout << "Replica: " << name() << ":Running distributed" << std::endl;
  } else {
    testServer.stop();
    std::cout << "Primary: " << name() << ":Running distributed" << std::endl;
  }

  if (hasCapability(CAP_AUDIO_IO)) {
    mAudioControl.registerAudioIO(audioIO());
  } else {
    mDomainList.erase(
        std::find(mDomainList.begin(), mDomainList.end(), mAudioDomain));
  }
  parameterServer() << mAudioControl.gain;
}

void DistributedApp::start() {
  initialize();
  initializeDomains();
  stdControls.app = this;

  if (hasCapability(CAP_OMNIRENDERING)) {
    omniRendering =
        graphicsDomain()->newSubDomain<GLFWOpenGLOmniRendererDomain>();
    omniRendering->initialize(graphicsDomain().get());
    omniRendering->window().append(stdControls);

    omniRendering->window().append(omniRendering->navControl());
    omniRendering->navControl().nav(omniRendering->nav());
    stdControls.mWindow = &omniRendering->window();
    omniRendering->onDraw =
        std::bind(&App::onDraw, this, std::placeholders::_1);

    omniRendering->window().onKeyDown =
        std::bind(&App::onKeyDown, this, std::placeholders::_1);
    omniRendering->window().onKeyUp =
        std::bind(&App::onKeyUp, this, std::placeholders::_1);
    omniRendering->window().onMouseDown =
        std::bind(&App::onMouseDown, this, std::placeholders::_1);
    omniRendering->window().onMouseUp =
        std::bind(&App::onMouseUp, this, std::placeholders::_1);
    omniRendering->window().onMouseDrag =
        std::bind(&App::onMouseDrag, this, std::placeholders::_1);
    omniRendering->window().onMouseMove =
        std::bind(&App::onMouseMove, this, std::placeholders::_1);
    omniRendering->window().onMouseScroll =
        std::bind(&App::onMouseScroll, this, std::placeholders::_1);
    if (!isPrimary()) {
      std::cout << "Running REPLICA" << std::endl;
      omniRendering->drawOmni = true;
    }
  } else if (hasCapability(CAP_RENDERING)) {

    std::cout << "Running Primary" << std::endl;
    mDefaultWindowDomain = graphicsDomain()->newWindow();
    mDefaultWindowDomain->initialize(graphicsDomain().get());
    mDefaultWindowDomain->window().append(stdControls);
    stdControls.app = this;
    stdControls.mWindow = &mDefaultWindowDomain->window();

    defaultWindow().append(mDefaultWindowDomain->navControl());

    mDefaultWindowDomain->onDraw =
        std::bind(&App::onDraw, this, std::placeholders::_1);
    mDefaultWindowDomain->window().onKeyDown =
        std::bind(&App::onKeyDown, this, std::placeholders::_1);
    mDefaultWindowDomain->window().onKeyUp =
        std::bind(&App::onKeyUp, this, std::placeholders::_1);
    mDefaultWindowDomain->window().onMouseDown =
        std::bind(&App::onMouseDown, this, std::placeholders::_1);
    mDefaultWindowDomain->window().onMouseUp =
        std::bind(&App::onMouseUp, this, std::placeholders::_1);
    mDefaultWindowDomain->window().onMouseDrag =
        std::bind(&App::onMouseDrag, this, std::placeholders::_1);
    mDefaultWindowDomain->window().onMouseMove =
        std::bind(&App::onMouseMove, this, std::placeholders::_1);
    mDefaultWindowDomain->window().onMouseScroll =
        std::bind(&App::onMouseScroll, this, std::placeholders::_1);
  }

  if (!isPrimary()) {
    mSimulationDomain
        ->disableProcessingCallback(); // Replicas won't call onAnimate()
  }

  onInit();

  for (auto &domain : mDomainList) {
    mRunningDomains.push(domain);
    if (!domain->start()) {
      std::cerr << "ERROR starting domain " << std::endl;
      break;
    }
  }

  while (mRunningDomains.size() > 0) {
    if (!mRunningDomains.top()->stop()) {
      std::cerr << "ERROR stopping domain " << std::endl;
    }
    mRunningDomains.pop();
  }

  onExit();
  mDefaultWindowDomain = nullptr;
  for (auto &domain : mDomainList) {
    if (!domain->cleanup()) {
      std::cerr << "ERROR cleaning up domain " << std::endl;
    }
  }
}

std::string DistributedApp::name() { return al_get_hostname(); }

void al::DistributedApp::registerDynamicScene(DynamicScene &scene) {
  if (dynamic_cast<DistributedScene *>(&scene)) {
    // If distributed scene, connect according to this app's role
    DistributedScene *s = dynamic_cast<DistributedScene *>(&scene);
    if (isPrimary()) {
      s->registerNotifier(parameterServer());
    } else {
      parameterServer().registerOSCConsumer(s, s->name());
    }
  }

  scene.prepare(audioIO());
}
