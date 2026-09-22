#ifndef AL_PARAM_SET_HPP
#define AL_PARAM_SET_HPP

/**
 * @file al_ParamSet.hpp
 * @brief Entity-scoped collection of ParameterMeta* (ECS-facing).
 *
 * Binding a param sets its group to the entity path so OSC addresses become
 * `/entityPath/paramName`. Capture/apply use ParamState (composition unit).
 */

#include <string>
#include <vector>

#include "al/parameter/al_ParamState.hpp"

namespace al {

class ParameterMeta;

class ParamSet {
public:
  ParamSet() = default;
  explicit ParamSet(std::string entityPath);

  void setEntityPath(std::string entityPath);
  const std::string &entityPath() const { return mEntityPath; }

  /// Register param and set its group to entityPath (rebuilds OSC address).
  ParamSet &add(ParameterMeta &param);

  ParamSet &operator<<(ParameterMeta &param) { return add(param); }

  const std::vector<ParameterMeta *> &parameters() const { return mParams; }
  std::vector<ParameterMeta *> &parameters() { return mParams; }

  ParamState capture() const;
  void apply(const ParamState &state);

  void clear() { mParams.clear(); }

private:
  std::string mEntityPath;
  std::vector<ParameterMeta *> mParams;
};

} // namespace al

#endif
