#include "al/parameter/al_KeyframeTrack.hpp"

#include <algorithm>
#include <cmath>

namespace al {

KeyframeTrack::KeyframeTrack(std::string name) : mName(std::move(name)) {}

double KeyframeTrack::easeUnit(Ease ease, double u) {
  if (u <= 0.0) {
    return 0.0;
  }
  if (u >= 1.0) {
    return 1.0;
  }
  switch (ease) {
  case Ease::Smoothstep:
    return u * u * (3.0 - 2.0 * u);
  case Ease::Smoothstep5:
    return u * u * u * (u * (u * 6.0 - 15.0) + 10.0);
  case Ease::Linear:
  default:
    return u;
  }
}

void KeyframeTrack::add(double time, ParamState state) {
  add(Keyframe{time, std::move(state)});
}

void KeyframeTrack::add(Keyframe kf) {
  auto it = std::lower_bound(
      mKeys.begin(), mKeys.end(), kf.time,
      [](const Keyframe &k, double t) { return k.time < t; });
  if (it != mKeys.end() && std::abs(it->time - kf.time) < 1e-12) {
    *it = std::move(kf);
    return;
  }
  mKeys.insert(it, std::move(kf));
}

double KeyframeTrack::duration() const {
  return mKeys.empty() ? 0.0 : mKeys.back().time;
}

ParamState KeyframeTrack::evaluate(double timeSeconds) const {
  if (mKeys.empty()) {
    return {};
  }
  if (timeSeconds <= mKeys.front().time) {
    return mKeys.front().state;
  }
  if (timeSeconds >= mKeys.back().time) {
    return mKeys.back().state;
  }

  // Find segment [i, i+1] with keys[i].time <= t < keys[i+1].time
  size_t i = 0;
  while (i + 1 < mKeys.size() && mKeys[i + 1].time <= timeSeconds) {
    ++i;
  }
  const Keyframe &a = mKeys[i];
  const Keyframe &b = mKeys[i + 1];
  double span = b.time - a.time;
  double u = span > 0.0 ? (timeSeconds - a.time) / span : 1.0;
  u = easeUnit(mEase, u);
  return lerpParamState(a.state, b.state, u);
}

void KeyframeTrack::apply(double timeSeconds,
                          const std::vector<ParameterMeta *> &targets) const {
  applyParamState(evaluate(timeSeconds), targets);
}

} // namespace al
