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
#include "al/io/al_PersistentConfig.hpp"
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

  Parameter gain{"gain", "sound", 1.0, 0.0, 2.0};
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

  /**
   * @brief prepares domains and configuration
   *
   * You can run this function manually to change domain behavior before the
   * domains are actually started, for example to disable state sharing on a
   * primary application.
   */
  virtual void prepare();

  std::string getPrimaryHost();

  void registerDynamicScene(DynamicScene &scene);

  Graphics &graphics() override;
  Window &defaultWindow() override;
  Viewpoint &view() override;
  Pose &pose() override;
  Lens &lens() override;
  Nav &nav() override;
  NavInputControl &navControl() override;

  void printCapabilities();

  std::shared_ptr<GLFWOpenGLOmniRendererDomain> omniRendering;
  std::map<std::string, std::string> additionalConfig;

private:
  AudioControl mAudioControl;

  bool initialized{false};

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

  void setPort(uint16_t port) {
    if (this->mRunningDomains.size() == 0) {
      std::cerr << __FUNCTION__
                << " ERROR can't set port while application"
                   "is running. Port will be applied on next call to start()."
                << std::endl;
    }
    mPortToSet = port;
  }

  void start() override {
    prepare();
    auto distDomain =
        std::static_pointer_cast<StateDistributionDomain<TSharedState>>(
            mSimulationDomain);
    if (isPrimary()) {
      if (hasCapability(CAP_STATE_SEND)) {
        auto sender =
            distDomain->addStateSender("state", distDomain->statePtr());
        sender->configure(mPortToSet, "state",
                          additionalConfig["broadcastAddress"]);
      } else {
        std::cout << "Not enabling state sending for primary." << std::endl;
      }
    } else {
      if (hasCapability(CAP_STATE_RECEIVE)) {
        auto receiver =
            distDomain->addStateReceiver("state", distDomain->statePtr());
        receiver->configure(mPortToSet, "state",
                            additionalConfig["broadcastAddress"]);
      }
    }
    DistributedApp::start();
  }

private:
  uint16_t mPortToSet{10101};
};

} // namespace al

#endif
