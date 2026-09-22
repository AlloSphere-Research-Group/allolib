#include "al/parameter/al_Entity.hpp"

#include "al/ui/al_Parameter.hpp"

namespace al {

Entity::Entity(EntityId id, std::string name)
    : mId(id), mName(std::move(name)) {
  syncParamPath();
}

void Entity::setId(EntityId id) {
  mId = id;
  syncParamPath();
}

void Entity::setName(std::string name) {
  mName = std::move(name);
  syncParamPath();
}

void Entity::setPath(std::string path) {
  mPath = std::move(path);
  mParams.setEntityPath(mPath);
}

Entity &Entity::addParam(ParameterMeta &param) {
  mParams.add(param);
  return *this;
}

void Entity::syncParamPath() {
  if (mPath.empty()) {
    if (!mName.empty()) {
      mPath = mName;
    } else if (mId != 0) {
      mPath = "e" + std::to_string(mId);
    }
  }
  if (!mPath.empty()) {
    mParams.setEntityPath(mPath);
  }
}

} // namespace al
