#ifndef AL_PARAM_NUMERIC_HPP
#define AL_PARAM_NUMERIC_HPP

/**
 * @file al_ParamNumeric.hpp
 * @brief Shared implementation for scalar numeric Parameter types.
 *
 * Include only after ParameterWrapper<T> is defined (from al_Parameter.hpp),
 * while still inside namespace al. Concrete types (ParameterInt,
 * ParameterDouble, …) are thin subclasses.
 *
 * Min/max are metadata for UI / optional Constraint filters — this class does
 * not clamp on set.
 */

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "al/types/al_VariantValue.hpp"

namespace detail {

template <typename T> struct ParamNumericDefaults {
  static T min() { return T(0); }
  static T max() { return std::numeric_limits<T>::max(); }
};

template <> struct ParamNumericDefaults<int32_t> {
  static int32_t min() { return 0; }
  static int32_t max() { return 127; }
};

template <> struct ParamNumericDefaults<double> {
  static double min() { return -99999.0; }
  static double max() { return 99999.0; }
};

template <typename T> inline T variantAs(const VariantValue &v) {
  return static_cast<T>(v.toDouble());
}

} // namespace detail

/**
 * @brief Scalar numeric parameter (lock-free store, matching ParameterInt).
 * @ingroup UI
 */
template <typename T> class ParamNumeric : public ParameterWrapper<T> {
public:
  using ParameterWrapper<T>::get;
  using ParameterWrapper<T>::set;
  using ParameterWrapper<T>::min;
  using ParameterWrapper<T>::max;
  using ParameterWrapper<T>::getDefault;
  using ParameterWrapper<T>::getFullAddress;
  using ParameterWrapper<T>::getName;
  using ParameterWrapper<T>::getGroup;

  ParamNumeric(std::string parameterName, std::string group = "",
               T defaultValue = T(),
               T minValue = detail::ParamNumericDefaults<T>::min(),
               T maxValue = detail::ParamNumericDefaults<T>::max())
      : ParameterWrapper<T>(std::move(parameterName), std::move(group),
                            defaultValue, minValue, maxValue) {
    this->mValue = defaultValue;
    this->setDefault(defaultValue);
  }

  ParamNumeric(const ParamNumeric &other) : ParameterWrapper<T>(other) {
    this->mValue = other.mValue;
    this->setDefault(other.getDefault());
  }

  void set(T value, ValueSource *src = nullptr) override {
    this->mValueCache = this->get();
    value = this->applySetFilters(value);
    this->mValue = value;
    this->runChangeCallbacksSynchronous(value, src);
  }

  void setNoCalls(T value, void *blockReceiver = nullptr) override {
    value = this->applySetFilters(value);
    this->mValue = value;
    this->mChanged = true;
    if (blockReceiver) {
      this->runChangeCallbacksSynchronous(value, nullptr);
    }
  }

  float toFloat() override { return static_cast<float>(this->get()); }

  bool fromFloat(float value) override {
    this->set(static_cast<T>(value));
    return true;
  }

  T operator=(T value) {
    this->set(value);
    return value;
  }

  void getFields(std::vector<VariantValue> &fields) override {
    fields.emplace_back(VariantValue(this->get()));
  }

  void setFields(std::vector<VariantValue> &fields) override {
    if (fields.size() == 1) {
      this->set(detail::variantAs<T>(fields[0]));
    }
  }

  void sendValue(osc::Send &sender, std::string prefix = "") override {
    sender.send(prefix + this->getFullAddress(), this->get());
  }

  void sendMeta(osc::Send &sender, std::string bundleName = "",
                std::string id = "") override {
    if (bundleName.empty()) {
      sender.send("/registerParameter", this->getName(), this->getGroup(),
                  this->getDefault(), std::string(), this->min(), this->max());
    } else {
      sender.send("/registerBundleParameter", bundleName, id, this->getName(),
                  this->getGroup(), this->getDefault(), std::string(),
                  this->min(), this->max());
    }
  }
};

#endif
