#include "al/parameter/al_ParamState.hpp"

#include "al/ui/al_Parameter.hpp"

namespace al {
namespace {

double variantAsDouble(const VariantValue &v) {
  switch (v.type()) {
  case VariantType::VARIANT_FLOAT:
    return v.get<float>();
  case VariantType::VARIANT_DOUBLE:
    return v.get<double>();
  case VariantType::VARIANT_INT32:
    return static_cast<double>(v.get<int32_t>());
  case VariantType::VARIANT_INT8:
    return static_cast<double>(v.get<int8_t>());
  case VariantType::VARIANT_INT16:
    return static_cast<double>(v.get<int16_t>());
  case VariantType::VARIANT_UINT8:
    return static_cast<double>(v.get<uint8_t>());
  case VariantType::VARIANT_UINT16:
    return static_cast<double>(v.get<uint16_t>());
  case VariantType::VARIANT_UINT32:
    return static_cast<double>(v.get<uint32_t>());
  default:
    return 0.0;
  }
}

bool isNumericVariant(VariantType t) {
  return t == VariantType::VARIANT_FLOAT || t == VariantType::VARIANT_DOUBLE ||
         t == VariantType::VARIANT_INT32 || t == VariantType::VARIANT_INT8 ||
         t == VariantType::VARIANT_INT16 || t == VariantType::VARIANT_UINT8 ||
         t == VariantType::VARIANT_UINT16 || t == VariantType::VARIANT_UINT32;
}

bool coerceFieldToMatch(const VariantValue &current, VariantValue &incoming) {
  if (current.type() == incoming.type()) {
    return true;
  }
  if (current.type() == VariantType::VARIANT_FLOAT &&
      incoming.type() == VariantType::VARIANT_INT32) {
    incoming = VariantValue(float(incoming.get<int32_t>()));
    return true;
  }
  if (current.type() == VariantType::VARIANT_INT32 &&
      incoming.type() == VariantType::VARIANT_FLOAT) {
    incoming = VariantValue(int32_t(incoming.get<float>()));
    return true;
  }
  if (current.type() == VariantType::VARIANT_FLOAT &&
      incoming.type() == VariantType::VARIANT_DOUBLE) {
    incoming = VariantValue(float(incoming.get<double>()));
    return true;
  }
  if (current.type() == VariantType::VARIANT_DOUBLE &&
      incoming.type() == VariantType::VARIANT_FLOAT) {
    incoming = VariantValue(double(incoming.get<float>()));
    return true;
  }
  if (current.type() == VariantType::VARIANT_DOUBLE &&
      incoming.type() == VariantType::VARIANT_INT32) {
    incoming = VariantValue(double(incoming.get<int32_t>()));
    return true;
  }
  if (current.type() == VariantType::VARIANT_INT32 &&
      incoming.type() == VariantType::VARIANT_DOUBLE) {
    incoming = VariantValue(int32_t(incoming.get<double>()));
    return true;
  }
  return false;
}

VariantValue lerpField(const VariantValue &start, const VariantValue &end,
                       double factor) {
  if (start.type() == VariantType::VARIANT_STRING) {
    return factor >= 1.0 ? end : start;
  }
  if (!isNumericVariant(start.type()) || !isNumericVariant(end.type())) {
    return factor >= 1.0 ? end : start;
  }
  double v = variantAsDouble(start) +
             factor * (variantAsDouble(end) - variantAsDouble(start));
  if (start.type() == VariantType::VARIANT_INT32) {
    return VariantValue(static_cast<int32_t>(v));
  }
  if (start.type() == VariantType::VARIANT_DOUBLE) {
    return VariantValue(v);
  }
  return VariantValue(static_cast<float>(v));
}

} // namespace

ParamState captureParamState(const std::vector<ParameterMeta *> &params) {
  ParamState state;
  for (ParameterMeta *p : params) {
    if (!p) {
      continue;
    }
    ParamFields fields;
    p->getFields(fields);
    state[p->getFullAddress()] = std::move(fields);
  }
  return state;
}

void applyParamState(const ParamState &state,
                     const std::vector<ParameterMeta *> &params) {
  for (ParameterMeta *p : params) {
    if (!p) {
      continue;
    }
    auto it = state.find(p->getFullAddress());
    if (it == state.end()) {
      continue;
    }
    ParamFields fields = it->second;
    ParamFields current;
    p->getFields(current);
    if (current.size() == fields.size()) {
      for (size_t i = 0; i < current.size(); ++i) {
        coerceFieldToMatch(current[i], fields[i]);
      }
    }
    p->setFields(fields);
  }
}

ParamState lerpParamState(const ParamState &a, const ParamState &b, double t) {
  ParamState out;
  if (t <= 0.0) {
    return a;
  }
  if (t >= 1.0) {
    // Prefer b's paths; hold a-only paths.
    out = a;
    for (const auto &kv : b) {
      out[kv.first] = kv.second;
    }
    return out;
  }

  for (const auto &kv : a) {
    auto it = b.find(kv.first);
    if (it == b.end() || it->second.size() != kv.second.size()) {
      out[kv.first] = kv.second; // hold
      continue;
    }
    ParamFields blended;
    blended.reserve(kv.second.size());
    for (size_t i = 0; i < kv.second.size(); ++i) {
      VariantValue end = it->second[i];
      coerceFieldToMatch(kv.second[i], end);
      blended.push_back(lerpField(kv.second[i], end, t));
    }
    out[kv.first] = std::move(blended);
  }
  return out;
}

} // namespace al
