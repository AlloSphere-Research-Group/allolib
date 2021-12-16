
#include <iostream>
#include <stdexcept>

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
  mType = VARIANT_CHAR;
  mData = new char;
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

VariantValue::VariantValue(const int16_t value) {
  mType = VARIANT_INT16;
  mData = new int16_t;
  *static_cast<int16_t *>(mData) = value;
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

VariantValue::VariantValue(const uint16_t value) {
  mType = VARIANT_UINT16;
  mData = new uint16_t;
  *static_cast<uint16_t *>(mData) = uint16_t(value);
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

VariantValue::VariantValue(const bool value) {
  mType = VARIANT_BOOL;
  mData = new bool; 
  *static_cast<bool *>(mData) = value;
}

VariantValue::~VariantValue() {
  // TODO ML implement all types. Done.
  switch (mType) {
  case VARIANT_FLOAT:
    delete static_cast<float *>(mData);
    break;
  case VARIANT_DOUBLE:
    delete static_cast<double *>(mData);
    break;
  case VARIANT_INT8:
    delete static_cast<int8_t *>(mData);
    break;
  case VARIANT_UINT8:
    delete static_cast<uint8_t *>(mData);
    break;
  case VARIANT_INT16:
    delete static_cast<int16_t *>(mData);
    break;
  case VARIANT_UINT16:
    delete static_cast<uint16_t *>(mData);
    break;
  case VARIANT_INT32:
    delete static_cast<int32_t *>(mData);
    break;
  case VARIANT_UINT32:
    delete static_cast<uint32_t *>(mData);
    break;
  case VARIANT_INT64:
    delete static_cast<int64_t *>(mData);
    break;
  case VARIANT_UINT64:
    delete static_cast<uint64_t *>(mData);
    break;
  case VARIANT_STRING:
    delete static_cast<std::string *>(mData);
    break;
  case VARIANT_BOOL:
    delete static_cast<bool *>(mData);
    break;
  case VARIANT_CHAR:
    delete static_cast<char *>(mData);
    break;
  case VARIANT_NONE:
    break;
  }
}

VariantValue::VariantValue(const VariantValue &paramField)
    : mType(paramField.mType) {
  // TODO ML complete all types. Done
  switch (mType) {
  case VARIANT_FLOAT:
    mData = new float;
    *static_cast<float *>(mData) = *static_cast<float *>(paramField.mData);
    break;
  case VARIANT_DOUBLE:
    mData = new double;
    *static_cast<double *>(mData) = *static_cast<double *>(paramField.mData);
    break;
  case VARIANT_STRING:
    mData = new std::string;
    *static_cast<std::string *>(mData) = *static_cast<std::string *>(paramField.mData);
    break;
  case VARIANT_INT8:
    mData = new int8_t;
    *static_cast<int8_t *>(mData) = *static_cast<int8_t *>(paramField.mData);
    break;
  case VARIANT_UINT8:
    mData = new uint8_t;
    *static_cast<uint8_t *>(mData) = *static_cast<uint8_t *>(paramField.mData);
    break;
  case VARIANT_INT16:
    mData = new int16_t;
    *static_cast<int16_t *>(mData) = *static_cast<int16_t *>(paramField.mData);
    break;
  case VARIANT_UINT16:
    mData = new uint16_t;
    *static_cast<uint16_t *>(mData) = *static_cast<uint16_t *>(paramField.mData);
    break;
  case VARIANT_INT32:
    mData = new int32_t;
    *static_cast<int32_t *>(mData) = *static_cast<int32_t *>(paramField.mData);
    break;
  case VARIANT_UINT32:
    mData = new uint32_t;
    *static_cast<uint32_t *>(mData) = *static_cast<uint32_t *>(paramField.mData);
    break;
  case VARIANT_INT64:
    mData = new int64_t;
    *static_cast<int64_t *>(mData) = *static_cast<int64_t *>(paramField.mData);
    break;
  case VARIANT_UINT64:
    mData = new uint64_t;
    *static_cast<uint64_t *>(mData) = *static_cast<uint64_t *>(paramField.mData);
    break;
  case VARIANT_CHAR:
    mData = new char;
    *static_cast<char *>(mData) =
        *static_cast<char *>(paramField.mData);
    break;
  case VARIANT_BOOL:
    mData = new bool;
    *static_cast<bool *>(mData) =
        *static_cast<bool *>(paramField.mData);
    break;    
  case VARIANT_NONE:
    break;
  default:
    throw std::invalid_argument("Data type not supported");
  }
}
