#ifndef AL_PARAM_STATE_HPP
#define AL_PARAM_STATE_HPP

/**
 * @file al_ParamState.hpp
 * @brief Path-keyed snapshot of parameter fields (composition / sync unit).
 *
 * A ParamState maps OSC path → field list (from ParameterMeta::getFields).
 * This is the shared currency for presets, keyframes, morph, and distributed
 * replication — independent of PresetHandler / SynthSequencer.
 */

#include <map>
#include <string>
#include <vector>

#include "al/types/al_VariantValue.hpp"

namespace al {

class ParameterMeta;

using ParamFields = std::vector<VariantValue>;

/// Snapshot of many parameters: full OSC address → fields.
using ParamState = std::map<std::string, ParamFields>;

/// Capture current fields for each param (keyed by getFullAddress()).
ParamState captureParamState(const std::vector<ParameterMeta *> &params);

/// Apply matching paths; unmatched paths in \p state are ignored.
void applyParamState(const ParamState &state,
                     const std::vector<ParameterMeta *> &params);

/**
 * @brief Linear blend of two snapshots (per field, per path).
 *
 * Paths only in \p a are held; only in \p b are taken from \p b at t>=1,
 * otherwise held from a. Matching paths lerp numeric fields; strings jump at
 * t>=1. Missing field counts skip that path.
 */
ParamState lerpParamState(const ParamState &a, const ParamState &b, double t);

} // namespace al

#endif
