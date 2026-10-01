#ifndef AL_ECS_SCENE_HPP
#define AL_ECS_SCENE_HPP

/**
 * @file al_Scene.hpp
 * @brief Entity Component Scene — pool, archetypes, render, dual sync paths.
 *
 * Supersedes DynamicScene / DistributedScene for new code. Distribution is
 * capability (packState / ParameterServer), not a subclass hierarchy.
 *
 * Sync pathways:
 * - Parameters (shareParameter) — event / OSC / composition
 * - packState / unpackState — per-frame POD mirror (primary → replicas)
 *
 * Audio:
 * - Default render path is dry channel-sum (no spatializer) — explicit opt-in
 *   via enableSpatialAudio<TSpatilizer>(Speakers) (unlike DynamicScene which
 *   always installs StereoPanner).
 */

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "al/graphics/al_Graphics.hpp"
#include "al/io/al_AudioIOData.hpp"
#include "al/protocol/al_OSC.hpp"
#include "al/scene/al_Entity.hpp"
#include "al/sound/al_Spatializer.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/spatial/al_DistAtten.hpp"
#include "al/spatial/al_Pose.hpp"
#include "al/types/al_SingleRWRingBuffer.hpp"
#include "al/types/al_TimeMasterMode.hpp"
#include "al/ui/al_ParameterServer.hpp"

namespace al {

class Scene : public osc::MessageConsumer {
public:
  using EntityFactory = std::function<void(Entity &)>;

  Scene(std::string name = "scene",
        TimeMasterMode masterMode = TimeMasterMode::TIME_MASTER_GRAPHICS);
  ~Scene();

  void registerEntity(const std::string &name, EntityFactory factory);
  void allocatePool(const std::string &name, int count);
  Entity *getEntity(const std::string &name, bool forceAlloc = false);

  int triggerOn(Entity *entity, int id = -1);
  void triggerOff(int id);
  void allNotesOff();

  void update(double dt = 0);
  void render(AudioIOData &io);
  void render(Graphics &g);

  size_t packState(char *buf, size_t maxSize) const;
  void unpackState(const char *buf, size_t size);

  std::string name() const { return mName; }

  void registerWithParameterServer(ParameterServer &server, bool isPrimary);
  void registerNotifier(OSCNotifier &notifier);
  bool consumeMessage(osc::Message &m,
                      std::string rootOSCPath = "") override;

  void setReplica(bool isReplica = true) { mIsReplica = isReplica; }
  bool isReplica() const { return mIsReplica; }

  void gain(float g) { mAudioGain = g; }
  float gain() const { return mAudioGain; }

  Pose &listenerPose() { return mListenerPose; }
  const Pose &listenerPose() const { return mListenerPose; }

  /// Explicit spatial audio (off by default — dry bus mix until enabled).
  template <class TSpatializer>
  std::shared_ptr<TSpatializer> enableSpatialAudio(const Speakers &sl) {
    auto spat = std::make_shared<TSpatializer>(sl);
    spat->compile();
    mSpatializer = spat;
    return spat;
  }

  template <class TSpatializer>
  std::shared_ptr<TSpatializer> enableSpatialAudio(const Speakers &&sl) {
    return enableSpatialAudio<TSpatializer>(sl);
  }

  void disableSpatialAudio() { mSpatializer.reset(); }
  bool spatialAudioEnabled() const { return static_cast<bool>(mSpatializer); }
  DistAtten<> &distanceAttenuation() { return mDistAtten; }
  const DistAtten<> &distanceAttenuation() const { return mDistAtten; }
  void useDistanceAttenuation(bool on) { mUseDistAtten = on; }
  bool useDistanceAttenuation() const { return mUseDistAtten; }

  void prepare(AudioIOData &io);
  void setEntityMaxOutputChannels(uint16_t channels) {
    mEntityMaxOutputChannels = channels;
  }

  void setTimeMaster(TimeMasterMode mode) { mMasterMode = mode; }
  TimeMasterMode timeMaster() const { return mMasterMode; }

  /// Find active (or pending insert) entity by id.
  Entity *findEntityById(int32_t id) const;

  /// Ids of currently active entities (for replica bookkeeping / UI).
  std::vector<int32_t> activeEntityIds() const;

  void print(std::ostream &stream = std::cout);

  static const size_t ENTITY_HEADER_SIZE = 4 + 2 + 12 + 16 + 4; // 38 bytes

private:
  void processInsertions();
  void processTurnOffs();
  void processInactive();
  void renderAudioDry(AudioIOData &io);
  void renderAudioSpatial(AudioIOData &io);

  Entity *allocateEntity(const std::string &name);

  void registerParameterCallbacks(Entity *entity);
  void registerCallbackForParameter(Entity *entity, ParameterMeta *param);

  struct ArchetypeInfo {
    std::string name;
    EntityFactory factory;
    size_t componentStateSize;
  };

  void rebuildArchetypeIndex();

  std::vector<ArchetypeInfo> mArchetypes;
  std::map<std::string, size_t> mArchetypeIndex;

  Entity *mActiveEntities{nullptr};
  Entity *mFreeEntities{nullptr};
  Entity *mEntitiesToInsert{nullptr};

  std::mutex mInsertLock;
  std::mutex mFreeLock;

  SingleRWRingBuffer mIdsToTurnOff{64 * sizeof(int)};

  std::string mName;
  OSCNotifier *mNotifier{nullptr};
  bool mIsReplica{false};

  Pose mListenerPose;
  float mAudioGain{1.0f};
  int mIdCounter{1000};
  bool mAllNotesOff{false};

  std::shared_ptr<Spatializer> mSpatializer;
  DistAtten<> mDistAtten;
  bool mUseDistAtten{true};

  TimeMasterMode mMasterMode;
  AudioIOData mInternalAudioIO;
  bool mAudioConfigured{false};
  uint16_t mEntityMaxOutputChannels{2};
};

} // namespace al

#endif
