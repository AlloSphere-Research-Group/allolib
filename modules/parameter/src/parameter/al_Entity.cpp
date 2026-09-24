#include "al/parameter/al_Entity.hpp"

#include "al/ui/al_Parameter.hpp"

namespace al {

ParamEntity::ParamEntity(EntityId id, std::string name)
    : mId(id), mName(std::move(name)) {
  syncParamPath();
}

void ParamEntity::setId(EntityId id) {
  mId = id;
  syncParamPath();
}

void ParamEntity::setName(std::string name) {
  mName = std::move(name);
  syncParamPath();
}

void ParamEntity::setPath(std::string path) {
  mPath = std::move(path);
  mParams.setEntityPath(mPath);
}

ParamEntity &ParamEntity::addParam(ParameterMeta &param) {
  mParams.add(param);
  return *this;
}

void ParamEntity::syncParamPath() {
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
