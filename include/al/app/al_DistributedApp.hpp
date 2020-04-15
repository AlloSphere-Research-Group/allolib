#ifndef INCLUDE_AL_DISTRIBUTEDAPP_HPP
#define INCLUDE_AL_DISTRIBUTEDAPP_HPP

/* Keehong Youn, 2017, younkeehong@gmail.com
 * Andres Cabrera, 2018, 2019, mantaraya36@gmail.com
 */

#include <iostream>
#include <map>

#include "al/app/al_App.hpp"
#include "al/app/al_NodeConfiguration.hpp"
#include "al/app/al_OmniRendererDomain.hpp"
#include "al/app/al_StateDistributionDomain.hpp"
#include "al/io/al_Socket.hpp"
#include "al/io/al_Toml.hpp"
#include "al/scene/al_DistributedScene.hpp"
#include "al/scene/al_DynamicScene.hpp"

namespace al {

class AudioControl {
public:
  void registerAudioIO(AudioIO &io) {
    gain.registerChangeCallback([&io](float value) { io.gain(value); });
  }

  void update(AudioIO &io) { gain = io.mGain; }

  Parameter gain{"gain", "sound", 1.0, "alloapp", 0.0, 2.0};
};

/**
 * @brief DistributedApp class
 * @ingroup App
 *
 * This class will try to read a file called "distributed_app.toml" in the
 * current directory to set roles and capabilities.
 *
 * The file should look like:
 *
@code
broadcastAddress = "192.168.10.255"
[[node]]
  host = "ar01.1g"
  rank = 0
  role = "desktop"
[[node]]
  host = "gr02"
  rank = 1
  role = "renderer"
[[node]]
  host = "gr03"
  rank = 1
  role = "renderer"
[[node]]
  host = "gr04"
  rank = 1
  role = "renderer"
@endcode

  * The broadcast address is used for state sending and the node list sets the
  * role and capabilities of the application if the hostname matches one of the
  * nodes listed.
  *
  * By default, if no configuration file is found, the application will be
  * primary if the primary port is available. if it is not, it will become a
  * secondary application communicating to a primary on localhost.
 */
class DistributedApp : public App, public NodeConfiguration {
public:
  void start() override;

  std::string name();

  virtual void init();

  void registerDynamicScene(DynamicScene &scene);

  Graphics &graphics() override;
  Window &defaultWindow() override;
  Viewpoint &view() override;
  Pose &pose() override;
  Lens &lens() override;
  Nav &nav() override;
  NavInputControl &navControl() override;

  std::shared_ptr<GLFWOpenGLOmniRendererDomain> omniRendering;
  std::map<std::string, std::string> additionalConfig;

private:
  AudioControl mAudioControl;

  std::map<std::string, std::string> mRoleMap;
  bool mFoundHost = false;
};

/**
 *
 * Provides a class to ditribute synchronous state. By default, OSC blobs are
 * used which are currently limited in size by the osc library, so it wil fail
 * even with small states. If cuttlebone is available through
 * al_ext/stateditribution you can have this app use cuttlebone instead.
 *
 * If you are feeling adventurous, you can modify oscpack to allow larger packet
 * buffer.
 *
 * Currently, the state simulation domain is injected as a subdomain of the
 * graphics domain i.e. it runs synchronously to graphics domain, calling
 * onAnimate() within the graphics loop. This architure will allow
 * running onAnimate() on a separate thrad if desired.
 */
template <class TSharedState>
class DistributedAppWithState : public DistributedApp {
public:
  DistributedAppWithState() : DistributedApp() {
    // State will be same memory for local, but will be synced on the network
    // for separate instances
    mOpenGLGraphicsDomain->removeSubDomain(simulationDomain());

    // Replace Simulation domain with state simulation domain
    mSimulationDomain =
        mOpenGLGraphicsDomain
            ->newSubDomain<StateDistributionDomain<TSharedState>>(true);
  }

  TSharedState &state() {
    return std::static_pointer_cast<StateDistributionDomain<TSharedState>>(
               mSimulationDomain)
        ->state();
  }

  void init() override {
    DistributedApp::init();
    auto distDomain =
        std::static_pointer_cast<StateDistributionDomain<TSharedState>>(
            mSimulationDomain);
    if (isPrimary()) {
      std::cout << "Running primary" << std::endl;
      auto sender = distDomain->addStateSender("state", distDomain->statePtr());
      sender->configure(10101, "state", additionalConfig["broadcastAddress"]);
    } else {
      std::cout << "Running REPLICA" << std::endl;
      auto receiver =
          distDomain->addStateReceiver("state", distDomain->statePtr());
      receiver->configure(10101);
    }
  }

private:
};

} // namespace al

#endif
