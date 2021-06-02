#include "al/types/al_VariantValue.hpp"

using namespace al;

VariantValue::VariantValue() {
  mData = nullptr;
  mType = VARIANT_NONE;
}

VariantValue::VariantValue(const std::string value) {
  mType = VARIANT_STRING;
  mData = new std::string;
  *static_cast<std::string *>(mData) = value;
}

VariantValue::VariantValue(const char *value) {
  mType = VARIANT_STRING;
  mData = new std::string;
  *static_cast<std::string *>(mData) = value;
}

VariantValue::VariantValue(const int64_t value) {
  mType = VARIANT_INT64;
  mData = new int64_t;
  *static_cast<int64_t *>(mData) = int64_t(value);
}

VariantValue::VariantValue(const int32_t value) {
  mType = VARIANT_INT32;
  mData = new int32_t;
  *static_cast<int32_t *>(mData) = value;
}

VariantValue::VariantValue(const int8_t value) {
  mType = VARIANT_INT8;
  mData = new int8_t;
  *static_cast<int8_t *>(mData) = int8_t(value);
}

VariantValue::VariantValue(const uint64_t value) {
  mType = VARIANT_UINT64;
  mData = new uint64_t;
  *static_cast<uint64_t *>(mData) = uint64_t(value);
}

VariantValue::VariantValue(const uint32_t value) {
  mType = VARIANT_UINT32;
  mData = new uint32_t;
  *static_cast<uint32_t *>(mData) = uint32_t(value);
}

VariantValue::VariantValue(const uint8_t value) {
  mType = VARIANT_UINT8;
  mData = new uint8_t;
  *static_cast<uint8_t *>(mData) = uint8_t(value);
}

VariantValue::VariantValue(const double value) {
  mType = VARIANT_DOUBLE;
  mData = new double;
  *static_cast<double *>(mData) = double(value);
}

VariantValue::VariantValue(const float value) {
  mType = VARIANT_FLOAT;
  mData = new float;
  *static_cast<float *>(mData) = value;
}
