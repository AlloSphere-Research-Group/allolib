#ifndef AL_WORLD_HPP
#define AL_WORLD_HPP

/**
 * @file al_Scene.hpp
 * @brief World — ParamEntity container with ParamState capture/apply.
 *
 * Lightweight parameter graph helper. The Entity Component Scene lives in
 * al/scene/al_Scene.hpp (al::Scene).
 */

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "al/parameter/al_Entity.hpp"
#include "al/parameter/al_ParamState.hpp"

namespace al {

class World {
public:
  World() = default;
  explicit World(std::string name);

  const std::string &name() const { return mName; }
  void setName(std::string name);

  ParamEntity &create(std::string entityName = "");

  ParamEntity *find(EntityId id);
  const ParamEntity *find(EntityId id) const;
  ParamEntity *findByName(const std::string &entityName);

  bool destroy(EntityId id);
  void clear();

  size_t size() const { return mEntities.size(); }

  std::vector<ParamEntity *> entities();
  std::vector<const ParamEntity *> entities() const;

  ParamState capture() const;
  void apply(const ParamState &state);

private:
  std::string entityPathFor(const std::string &entityName) const;

  std::string mName;
  EntityId mNextId{1};
  std::unordered_map<EntityId, std::unique_ptr<ParamEntity>> mEntities;
};

} // namespace al

#endif
