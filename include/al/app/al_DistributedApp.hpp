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
 */
class DistributedApp : public App, public NodeConfiguration {
 public:
  void start() override;

  std::string name();

  void initialize();

  void registerDynamicScene(DynamicScene &scene);

  Graphics &graphics() override;
  Window &defaultWindow() override;
  Viewpoint &view() override;
  Pose &pose() override;
  Lens &lens() override;
  Nav &nav() override;

  std::shared_ptr<GLFWOpenGLOmniRendererDomain> omniRendering;
  std::map<std::string, std::string> additionalConfig;

 private:
  AudioControl mAudioControl;

  std::map<std::string, std::string> mRoleMap;
};

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

    if (rank == 0) {
      std::cout << "Running primary" << std::endl;
      auto sender =
          std::static_pointer_cast<StateDistributionDomain<TSharedState>>(
              mSimulationDomain)
              ->addStateSender("state");
      sender->configure(10101);
    } else {
      std::cout << "Running REPLICA" << std::endl;
      auto receiver =
          std::static_pointer_cast<StateDistributionDomain<TSharedState>>(
              mSimulationDomain)
              ->addStateReceiver("state");
      receiver->configure(10101);
      mSimulationDomain
          ->disableProcessingCallback();  // Replicas won't call onAnimate()
    }
  }

  TSharedState &state() {
    return std::static_pointer_cast<StateDistributionDomain<TSharedState>>(
               mSimulationDomain)
        ->state();
  }

  void setTitle(std::string title) { defaultWindow().title(title); }

 private:
};

}  // namespace al

#endif
