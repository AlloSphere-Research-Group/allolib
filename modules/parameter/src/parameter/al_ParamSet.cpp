#include "al/parameter/al_ParamSet.hpp"

#include "al/ui/al_Parameter.hpp"

namespace al {

ParamSet::ParamSet(std::string entityPath)
    : mEntityPath(std::move(entityPath)) {}

void ParamSet::setEntityPath(std::string entityPath) {
  mEntityPath = std::move(entityPath);
  for (ParameterMeta *p : mParams) {
    if (p) {
      p->setGroup(mEntityPath);
    }
  }
}

ParamSet &ParamSet::add(ParameterMeta &param) {
  if (!mEntityPath.empty()) {
    param.setGroup(mEntityPath);
  }
  mParams.push_back(&param);
  return *this;
}

ParamState ParamSet::capture() const { return captureParamState(mParams); }

void ParamSet::apply(const ParamState &state) {
  applyParamState(state, mParams);
}

} // namespace al
