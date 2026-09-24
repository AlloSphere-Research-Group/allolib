#ifndef AL_PARAM_ENTITY_HPP
#define AL_PARAM_ENTITY_HPP

/**
 * @file al_Entity.hpp
 * @brief ParamEntity: path + Pose + ParamSet (parameter-layer helper).
 *
 * Prefer al::Entity (al/scene/al_Entity.hpp) for ECS composition. ParamEntity
 * remains for ParamState capture without components (e.g. voice bridges).
 */

#include <cstdint>
#include <string>

#include "al/parameter/al_ParamSet.hpp"
#include "al/spatial/al_Pose.hpp"

namespace al {

using EntityId = uint64_t;

class ParamEntity {
public:
  ParamEntity() = default;
  ParamEntity(EntityId id, std::string name);

  EntityId id() const { return mId; }
  const std::string &name() const { return mName; }
  const std::string &path() const { return mPath; }

  void setId(EntityId id);
  void setName(std::string name);
  void setPath(std::string path);

  Pose &pose() { return mPose; }
  const Pose &pose() const { return mPose; }

  ParamSet &params() { return mParams; }
  const ParamSet &params() const { return mParams; }

  ParamEntity &addParam(ParameterMeta &param);

private:
  void syncParamPath();

  EntityId mId{0};
  std::string mName;
  std::string mPath;
  Pose mPose;
  ParamSet mParams;
};

} // namespace al

#endif
