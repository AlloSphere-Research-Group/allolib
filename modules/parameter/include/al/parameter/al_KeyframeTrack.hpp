#ifndef AL_KEYFRAME_TRACK_HPP
#define AL_KEYFRAME_TRACK_HPP

/**
 * @file al_KeyframeTrack.hpp
 * @brief Timed ParamState list with easing — core of composition (Phase 4).
 *
 * A track is a sequence of keyframes (time + ParamState). evaluate(t) lerps
 * between surrounding keys; apply(t, targets) writes into live parameters.
 * Intended to supersede ad-hoc PresetSequencer morph for entity params.
 */

#include <string>
#include <vector>

#include "al/parameter/al_ParamState.hpp"

namespace al {

class ParameterMeta;

enum class Ease {
  Linear,
  Smoothstep, ///< 3t^2 - 2t^3
  Smoothstep5 ///< 6t^5 - 15t^4 + 10t^3
};

struct Keyframe {
  double time{0.0}; ///< Seconds from track start
  ParamState state;
};

class KeyframeTrack {
public:
  KeyframeTrack() = default;
  explicit KeyframeTrack(std::string name);

  const std::string &name() const { return mName; }
  void setName(std::string name) { mName = std::move(name); }

  Ease ease() const { return mEase; }
  void setEase(Ease ease) { mEase = ease; }

  void clear() { mKeys.clear(); }
  size_t size() const { return mKeys.size(); }
  const std::vector<Keyframe> &keys() const { return mKeys; }

  /// Insert keyframe; keeps keys sorted by time. Same time replaces.
  void add(double time, ParamState state);
  void add(Keyframe kf);

  /// End time of last key (0 if empty).
  double duration() const;

  /// Interpolated state at \p timeSeconds (clamped to [0, duration]).
  ParamState evaluate(double timeSeconds) const;

  /// evaluate + applyParamState onto \p targets.
  void apply(double timeSeconds,
             const std::vector<ParameterMeta *> &targets) const;

  /// Map unit interval u∈[0,1] through ease curve.
  static double easeUnit(Ease ease, double u);

private:
  std::string mName;
  Ease mEase{Ease::Linear};
  std::vector<Keyframe> mKeys;
};

} // namespace al

#endif
