#include "al/parameter/al_Scene.hpp"

namespace al {

World::World(std::string name) : mName(std::move(name)) {}

void World::setName(std::string name) { mName = std::move(name); }

std::string World::entityPathFor(const std::string &entityName) const {
  if (mName.empty()) {
    return entityName;
  }
  if (entityName.empty()) {
    return mName;
  }
  return mName + "/" + entityName;
}

ParamEntity &World::create(std::string entityName) {
  EntityId id = mNextId++;
  if (entityName.empty()) {
    entityName = "e" + std::to_string(id);
  }
  auto entity = std::make_unique<ParamEntity>(id, entityName);
  entity->setPath(entityPathFor(entityName));
  ParamEntity &ref = *entity;
  mEntities[id] = std::move(entity);
  return ref;
}

ParamEntity *World::find(EntityId id) {
  auto it = mEntities.find(id);
  return it == mEntities.end() ? nullptr : it->second.get();
}

const ParamEntity *World::find(EntityId id) const {
  auto it = mEntities.find(id);
  return it == mEntities.end() ? nullptr : it->second.get();
}

ParamEntity *World::findByName(const std::string &entityName) {
  for (auto &kv : mEntities) {
    if (kv.second->name() == entityName) {
      return kv.second.get();
    }
  }
  return nullptr;
}

bool World::destroy(EntityId id) { return mEntities.erase(id) > 0; }

void World::clear() {
  mEntities.clear();
  mNextId = 1;
}

std::vector<ParamEntity *> World::entities() {
  std::vector<ParamEntity *> out;
  out.reserve(mEntities.size());
  for (auto &kv : mEntities) {
    out.push_back(kv.second.get());
  }
  return out;
}

std::vector<const ParamEntity *> World::entities() const {
  std::vector<const ParamEntity *> out;
  out.reserve(mEntities.size());
  for (const auto &kv : mEntities) {
    out.push_back(kv.second.get());
  }
  return out;
}

ParamState World::capture() const {
  ParamState state;
  for (const auto &kv : mEntities) {
    ParamState part = kv.second->params().capture();
    state.insert(part.begin(), part.end());
  }
  return state;
}

void World::apply(const ParamState &state) {
  for (auto &kv : mEntities) {
    kv.second->params().apply(state);
  }
}

} // namespace al
