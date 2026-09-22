#ifndef AL_SCENE_ECS_HPP
#define AL_SCENE_ECS_HPP

/**
 * @file al_Scene.hpp
 * @brief Entity container (the ECS "world") — destined to supersede DynamicScene.
 *
 * Named Scene to match AlloLib vocabulary; World is an alias for ECS familiarity.
 * Owns Entities, assigns ids, and can capture/apply a full ParamState snapshot.
 */

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "al/parameter/al_Entity.hpp"
#include "al/parameter/al_ParamState.hpp"

namespace al {

class Scene {
public:
  Scene() = default;
  explicit Scene(std::string name);

  const std::string &name() const { return mName; }
  void setName(std::string name);

  /// Create entity with unique id; path becomes `sceneName/entityName` when
  /// the scene is named.
  Entity &create(std::string entityName = "");

  Entity *find(EntityId id);
  const Entity *find(EntityId id) const;
  Entity *findByName(const std::string &entityName);

  bool destroy(EntityId id);
  void clear();

  size_t size() const { return mEntities.size(); }

  std::vector<Entity *> entities();
  std::vector<const Entity *> entities() const;

  /// Snapshot all parameters on all entities (keyed by full OSC path).
  ParamState capture() const;

  /// Apply state to all entity params (unmatched paths ignored).
  void apply(const ParamState &state);

private:
  std::string entityPathFor(const std::string &entityName) const;

  std::string mName;
  EntityId mNextId{1};
  std::unordered_map<EntityId, std::unique_ptr<Entity>> mEntities;
};

/// ECS-oriented name for the same type.
using World = Scene;

} // namespace al

#endif
