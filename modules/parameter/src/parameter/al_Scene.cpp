#include "al/parameter/al_Scene.hpp"

namespace al {

Scene::Scene(std::string name) : mName(std::move(name)) {}

void Scene::setName(std::string name) { mName = std::move(name); }

std::string Scene::entityPathFor(const std::string &entityName) const {
  if (mName.empty()) {
    return entityName;
  }
  if (entityName.empty()) {
    return mName;
  }
  return mName + "/" + entityName;
}

Entity &Scene::create(std::string entityName) {
  EntityId id = mNextId++;
  if (entityName.empty()) {
    entityName = "e" + std::to_string(id);
  }
  auto entity = std::make_unique<Entity>(id, entityName);
  entity->setPath(entityPathFor(entityName));
  Entity &ref = *entity;
  mEntities[id] = std::move(entity);
  return ref;
}

Entity *Scene::find(EntityId id) {
  auto it = mEntities.find(id);
  return it == mEntities.end() ? nullptr : it->second.get();
}

const Entity *Scene::find(EntityId id) const {
  auto it = mEntities.find(id);
  return it == mEntities.end() ? nullptr : it->second.get();
}

Entity *Scene::findByName(const std::string &entityName) {
  for (auto &kv : mEntities) {
    if (kv.second->name() == entityName) {
      return kv.second.get();
    }
  }
  return nullptr;
}

bool Scene::destroy(EntityId id) { return mEntities.erase(id) > 0; }

void Scene::clear() {
  mEntities.clear();
  mNextId = 1;
}

std::vector<Entity *> Scene::entities() {
  std::vector<Entity *> out;
  out.reserve(mEntities.size());
  for (auto &kv : mEntities) {
    out.push_back(kv.second.get());
  }
  return out;
}

std::vector<const Entity *> Scene::entities() const {
  std::vector<const Entity *> out;
  out.reserve(mEntities.size());
  for (const auto &kv : mEntities) {
    out.push_back(kv.second.get());
  }
  return out;
}

ParamState Scene::capture() const {
  ParamState state;
  for (const auto &kv : mEntities) {
    ParamState part = kv.second->params().capture();
    state.insert(part.begin(), part.end());
  }
  return state;
}

void Scene::apply(const ParamState &state) {
  for (auto &kv : mEntities) {
    kv.second->params().apply(state);
  }
}

} // namespace al
