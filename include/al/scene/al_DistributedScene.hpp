#ifndef AL_DISTRIBUTEDSCENE_HPP
#define AL_DISTRIBUTEDSCENE_HPP

#include "al/protocol/al_OSC.hpp"
#include "al/scene/al_DynamicScene.hpp"
#include "al/ui/al_ParameterServer.hpp"

namespace al {
/**
 * @brief The DistributedScene class
 * @ingroup Scene
 *
 * This scene can be registered with a ParameterServer to replicate the scene
 * across the network. All events and internal parameters will be forwarded
 * to scene copies
 */
class DistributedScene : public DynamicScene, public osc::MessageConsumer {
public:
  DistributedScene(std::string name = "scene", int threadPoolSize = 0,
                   TimeMasterMode masterMode = TimeMasterMode::TIME_MASTER_CPU);

  DistributedScene(TimeMasterMode masterMode)
      : DistributedScene("scene", 0, masterMode) {}

  std::string name() { return mName; }

  void registerNotifier(OSCNotifier &notifier) { mNotifier = &notifier; }

  virtual void allNotesOff() override;

  virtual bool consumeMessage(osc::Message &m,
                              std::string rootOSCPath = "") override;

protected:
private:
  OSCNotifier *mNotifier{nullptr};
  std::string mName;
};

} // namespace al

#endif // AL_DISTRIBUTEDSCENE_HPP
