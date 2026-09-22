#ifndef AL_ENTITY_HPP
#define AL_ENTITY_HPP

/**
 * @file al_Entity.hpp
 * @brief Minimal entity: id, path, default Pose, and a ParamSet.
 *
 * Owned by Scene (aka World). Voices/DynamicScene remain until Scene grows;
 * bind via al/scene/al_VoiceEntity.hpp.
 */

#include <cstdint>
#include <string>

#include "al/parameter/al_ParamSet.hpp"
#include "al/spatial/al_Pose.hpp"

namespace al {

using EntityId = uint64_t;

class Entity {
public:
  Entity() = default;
  Entity(EntityId id, std::string name);

  EntityId id() const { return mId; }
  const std::string &name() const { return mName; }

  /// Hierarchical path used as parameter group, e.g. "scene/actor3".
  const std::string &path() const { return mPath; }

  void setId(EntityId id);
  void setName(std::string name);
  /// Sets path and rebinds ParamSet groups (e.g. "world/e12").
  void setPath(std::string path);

  Pose &pose() { return mPose; }
  const Pose &pose() const { return mPose; }

  ParamSet &params() { return mParams; }
  const ParamSet &params() const { return mParams; }

  /// Convenience: add param under this entity's path.
  Entity &addParam(ParameterMeta &param);

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
