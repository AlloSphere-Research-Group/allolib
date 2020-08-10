#include "al/app/al_DistributedApp.hpp"

#include "al/sphere/al_SphereUtils.hpp"

#ifdef AL_WINDOWS
//#include <Windows.h>
#include <WinSock2.h>
#endif

using namespace al;

void DistributedApp::prepare() {
  if (initialized) {
    return;
  }
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

  if (!File::exists("distributed_app.toml")) {
    if (al::sphere::isSphereMachine()) {
      // If on sphere, create default config file
      std::ofstream configfile;
      configfile.open("distributed_app.toml");
      configfile << "broadcastAddress = \"192.168.10.255\"" << std::endl;

      configfile << "[[node]]" << std::endl;
      configfile << "host = \"ar01.1g\"\n";
      configfile << "rank = 0\n";
      configfile << "group = 0\n";
      configfile << "role = \"desktop\"\n\n";
      for (uint16_t i = 1; i <= 14; i++) {
        configfile << "[[node]]" << std::endl;
        char str[3];
        snprintf(str, 3, "%02d", i);
        configfile << "host = \"gr" + std::string(str) + "\"\n";
        configfile << "rank = " + std::to_string(i) + "\n";
        configfile << "group = 0\n";
        configfile << "role = \"renderer\"\n\n";
      }
      configfile.close();
    }
  }

  TomlLoader appConfig("distributed_app.toml");
  auto nodesTable = appConfig.root->get_table_array("node");
  std::vector<std::string> mListeners;
  // First get role from config file
  mFoundHost = false;
  if (nodesTable) {
    for (const auto &table : *nodesTable) {
      std::string host = *table->get_as<std::string>("host");
      std::string role = *table->get_as<std::string>("role");

      if (name() == host) {
        // Now set capabilities from role
        setRole(role);
        mFoundHost = true;
      }
      mRoleMap[host] = role;
      if (table->contains("dataRoot")) {
        if (name() == host) { // Set configuration for this node when found
          std::string dataRootValue = *table->get_as<std::string>("dataRoot");
          dataRoot = File::conformPathToOS(dataRootValue);
        }
      } else {
        std::cout << "WARNING: node " << host.c_str() << " not given dataRoot"
                  << std::endl;
      }

      if (table->contains("rank")) {
        if (name() == host) { // Set configuration for this node when found
          rank = *table->get_as<int>("rank");
        }
      } else {
        std::cout << "WARNING: node " << host.c_str() << " not given rank"
                  << std::endl;
      }
      if (table->contains("group")) {
        if (name() == host) { // Set configuration for this node when found
          group = *table->get_as<int>("group");
        }
      } else {
        std::cout << "WARNING: node " << host.c_str() << " not given group"
                  << std::endl;
      }
    }
    if (!mFoundHost) { // if host name isn't found, use default settings
                       // and warn
      std::cout
          << "WARNING: node " << name()
          << " not found in node table!\n\t*Using default desktop setting!*"
          << std::endl;
      setRole("desktop");
      rank = 0;
      group = 0;
    }
  } else { // No nodes table in config file. Use desktop role
    auto defaultCapabilities = al::sphere::getSphereNodes();
    if (defaultCapabilities.find(name()) != defaultCapabilities.end()) {
      mCapabilites = defaultCapabilities[name()].mCapabilites;
      group = defaultCapabilities[name()].group;
      rank = defaultCapabilities[name()].rank;
    } else {
      setRole("desktop");
      rank = 0;
      group = 0;
    }
  }

  if (hasCapability(CAP_SIMULATOR)) {
    if (al::sphere::isSphereMachine()) {
      appConfig.setDefaultValue("broadcastAddress",
                                std::string("192.168.10.255"));
      appConfig.writeFile();
    }
  }
  if (appConfig.hasKey<std::string>("broadcastAddress")) {
    if (mFoundHost) {
      additionalConfig["broadcastAddress"] = appConfig.gets("broadcastAddress");
    } else {
      additionalConfig["broadcastAddress"] = "127.0.0.1";
    }
  } else {
    additionalConfig["broadcastAddress"] = "127.0.0.1";
  }

  osc::Recv testServer;
  // probe to check if first port available, this will determine if this
  // application is the primary or the replica
  if (!testServer.open(mOSCDomain->port, mOSCDomain->interfaceIP.c_str())) {
    // If port taken, run this instance as a renderer
    mCapabilites = (Capability)(CAP_SIMULATOR | CAP_OMNIRENDERING | CAP_OSC);
    rank = 99;
    std::cout << "Replica: " << name() << ":Running distributed" << std::endl;
  } else if (rank == 0) {
    testServer.stop();
    std::cout << "Primary: " << name() << ":Running distributed" << std::endl;
  } else {
    testServer.stop();
    std::cout << "Secondary: rank " << rank << std::endl;
  }

  if (hasCapability(CAP_AUDIO_IO)) {
    mAudioControl.registerAudioIO(audioIO());
  } else {
    mDomainList.erase(
        std::find(mDomainList.begin(), mDomainList.end(), mAudioDomain));
  }
  parameterServer() << mAudioControl.gain;

  initializeDomains();
  initialized = true;
}

void DistributedApp::start() {
  prepare();
  stdControls.app = this;

  if (hasCapability(CAP_OMNIRENDERING)) {
    omniRendering =
        graphicsDomain()->newSubDomain<GLFWOpenGLOmniRendererDomain>();
    omniRendering->init(graphicsDomain().get());
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
      omniRendering->drawOmni = true;
    }
  } else if (hasCapability(CAP_RENDERING)) {
    mDefaultWindowDomain = graphicsDomain()->newWindow();
    mDefaultWindowDomain->init(graphicsDomain().get());
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

  if (isPrimary()) {
    std::cout << "Running Primary" << std::endl;
    if (!mFoundHost) {
      std::cout << "WARNING: not adding extra listeners due to missing node "
                   "table in distributed_app.toml"
                << std::endl;
    } else {
      std::cout << "Running REPLICA" << std::endl;
      for (auto hostRole : mRoleMap) {
        if (hostRole.first != name()) {
          parameterServer().addListener(hostRole.first, oscDomain()->port);
        }
      }
    }
    parameterServer().notifyAll();
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

Graphics &DistributedApp::graphics() {
  if (hasCapability(CAP_OMNIRENDERING)) {
    return omniRendering->graphics();
  } else {
    return mDefaultWindowDomain->graphics();
  }
}

Window &DistributedApp::defaultWindow() {
  if (hasCapability(CAP_OMNIRENDERING)) {
    return omniRendering->window();
  } else {
    return mDefaultWindowDomain->window();
  }
}

Viewpoint &DistributedApp::view() {
  if (hasCapability(CAP_OMNIRENDERING)) {
    return omniRendering->view();
  } else {
    return mDefaultWindowDomain->view();
  }
}

Pose &DistributedApp::pose() {
  if (hasCapability(CAP_OMNIRENDERING)) {
    return omniRendering->nav();
  } else {
    return mDefaultWindowDomain->nav();
  }
}

Lens &DistributedApp::lens() { return view().lens(); }

Nav &DistributedApp::nav() {
  if (hasCapability(CAP_OMNIRENDERING)) {
    return omniRendering->nav();
  } else {
    return mDefaultWindowDomain->nav();
  }
}

NavInputControl &DistributedApp::navControl() {
  if (hasCapability(CAP_OMNIRENDERING)) {
    return omniRendering->navControl();
  } else {
    return mDefaultWindowDomain->navControl();
  }
}
