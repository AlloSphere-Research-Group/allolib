#ifndef AL_SCENE_ENTITY_HPP
#define AL_SCENE_ENTITY_HPP

/**
 * @file al_Entity.hpp
 * @brief Composable scene object: id + Pose + Components (+ Parameters).
 *
 * Entity is nearly behavior-free; Components define how it looks, moves, and
 * sounds. Parameters on the entity (often owned by components) are the
 * event-driven control path; StatefulComponent PODs are the per-frame path.
 */

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "al/parameter/al_ParamState.hpp"
#include "al/scene/al_Component.hpp"
#include "al/spatial/al_Pose.hpp"
#include "al/ui/al_Parameter.hpp"

namespace al {

class Scene;

class Entity {
  friend class Scene;

public:
  Entity() = default;
  ~Entity() = default;

  Entity(const Entity &) = delete;
  Entity &operator=(const Entity &) = delete;

  template <class T, class... Args> T &add(Args &&...args) {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from Component");
    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    ptr->entity = this;
    T &ref = *ptr;
    mComponents.push_back(std::move(ptr));
    return ref;
  }

  template <class T> T *get() {
    for (auto &c : mComponents) {
      if (auto *p = dynamic_cast<T *>(c.get()))
        return p;
    }
    return nullptr;
  }

  template <class T> const T *get() const {
    for (auto &c : mComponents) {
      if (auto *p = dynamic_cast<const T *>(c.get()))
        return p;
    }
    return nullptr;
  }

  /// Remove first component of type T. Returns true if removed.
  template <class T> bool remove() {
    for (auto it = mComponents.begin(); it != mComponents.end(); ++it) {
      if (dynamic_cast<T *>(it->get())) {
        mComponents.erase(it);
        return true;
      }
    }
    return false;
  }

  const std::vector<std::unique_ptr<Component>> &components() const {
    return mComponents;
  }

  Pose &pose() { return mPose; }
  const Pose &pose() const { return mPose; }
  void setPose(const Pose &p) { mPose = p; }

  float size() const { return mSize; }
  void size(float s) { mSize = s; }

  bool active() const { return mActive; }
  int id() const { return mId; }
  const std::string &archetype() const { return mArchetype; }

  bool isPrimary() const { return !mIsReplica; }
  bool isReplica() const { return mIsReplica; }

  Entity &registerParameter(ParameterMeta &param) {
    mParameters.push_back(&param);
    return *this;
  }

  template <class... Args>
  Entity &registerParameters(Args &...paramsArgs) {
    std::vector<ParameterMeta *> params{&paramsArgs...};
    for (auto *p : params)
      mParameters.push_back(p);
    return *this;
  }

  std::vector<ParameterMeta *> &parameters() { return mParameters; }
  const std::vector<ParameterMeta *> &parameters() const { return mParameters; }

  Entity &shareParameter(ParameterMeta &param) {
    mParameters.push_back(&param);
    mSharedParameters.push_back(&param);
    return *this;
  }

  template <class... Args>
  Entity &shareParameters(Args &...paramsArgs) {
    std::vector<ParameterMeta *> params{&paramsArgs...};
    for (auto *p : params)
      shareParameter(*p);
    return *this;
  }

  std::vector<ParameterMeta *> &sharedParameters() { return mSharedParameters; }

  /// Flatten registered parameters into a ParamState (composition / OSC unit).
  ParamState captureParamState() const {
    return al::captureParamState(mParameters);
  }

  void applyParamState(const ParamState &state) {
    al::applyParamState(state, mParameters);
  }

  unsigned int numOutChannels() const { return mNumOutChannels; }
  void setNumOutChannels(unsigned int n) { mNumOutChannels = n; }

  void free() { mActive = false; }

  size_t componentStateSize() const {
    size_t total = 0;
    for (auto &c : mComponents)
      total += c->stateSize();
    return total;
  }

  size_t packComponentState(char *buf) const {
    size_t offset = 0;
    for (auto &c : mComponents) {
      size_t sz = c->stateSize();
      if (sz > 0) {
        c->packState(buf + offset);
        offset += sz;
      }
    }
    return offset;
  }

  size_t unpackComponentState(const char *buf) {
    size_t offset = 0;
    for (auto &c : mComponents) {
      size_t sz = c->stateSize();
      if (sz > 0) {
        c->unpackState(buf + offset);
        offset += sz;
      }
    }
    return offset;
  }

private:
  void doInit() {
    for (auto &c : mComponents)
      c->init();
  }

  void doTriggerOn() {
    mActive = true;
    for (auto &c : mComponents)
      c->onTriggerOn();
  }

  void doTriggerOff() {
    for (auto &c : mComponents)
      c->onTriggerOff();
  }

  void doUpdate(double dt) {
    for (auto &c : mComponents)
      c->update(dt);
  }

  void doAudio(AudioIOData &io) {
    for (auto &c : mComponents)
      c->onAudio(io);
  }

  void doDraw(Graphics &g) {
    for (auto &c : mComponents)
      c->onDraw(g);
  }

  void doFree() {
    for (auto &c : mComponents)
      c->onFree();
  }

  std::vector<std::unique_ptr<Component>> mComponents;
  std::vector<ParameterMeta *> mParameters;
  std::vector<ParameterMeta *> mSharedParameters;
  Pose mPose;
  float mSize{1.0f};
  bool mActive{false};
  bool mIsReplica{false};
  int mId{-1};
  unsigned int mNumOutChannels{1};
  std::string mArchetype;
  Entity *mNext{nullptr};
};

} // namespace al

#endif
