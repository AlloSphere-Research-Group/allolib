
#include <iostream>
#include <stdexcept>

#include "al/types/al_VariantValue.hpp"

using namespace al;

VariantValue::VariantValue() {
  mData = nullptr;
  mType = VariantType::VARIANT_NONE;
}

VariantValue::VariantValue(const std::string value) {
  mType = VariantType::VARIANT_STRING;
  mData = new std::string;
  *static_cast<std::string *>(mData) = value;
}

VariantValue::VariantValue(const char *value) {
  mType = VariantType::VARIANT_CHAR;
  mData = new char;
  *static_cast<std::string *>(mData) = value;
}

VariantValue::VariantValue(const int64_t value) {
  mType = VariantType::VARIANT_INT64;
  mData = new int64_t;
  *static_cast<int64_t *>(mData) = int64_t(value);
}

VariantValue::VariantValue(const int32_t value) {
  mType = VariantType::VARIANT_INT32;
  mData = new int32_t;
  *static_cast<int32_t *>(mData) = value;
}

VariantValue::VariantValue(const int16_t value) {
  mType = VariantType::VARIANT_INT16;
  mData = new int16_t;
  *static_cast<int16_t *>(mData) = value;
}

VariantValue::VariantValue(const int8_t value) {
  mType = VariantType::VARIANT_INT8;
  mData = new int8_t;
  *static_cast<int8_t *>(mData) = int8_t(value);
}

VariantValue::VariantValue(const uint64_t value) {
  mType = VariantType::VARIANT_UINT64;
  mData = new uint64_t;
  *static_cast<uint64_t *>(mData) = uint64_t(value);
}

VariantValue::VariantValue(const uint32_t value) {
  mType = VariantType::VARIANT_UINT32;
  mData = new uint32_t;
  *static_cast<uint32_t *>(mData) = uint32_t(value);
}

VariantValue::VariantValue(const uint16_t value) {
  mType = VariantType::VARIANT_UINT16;
  mData = new uint16_t;
  *static_cast<uint16_t *>(mData) = uint16_t(value);
}

VariantValue::VariantValue(const uint8_t value) {
  mType = VariantType::VARIANT_UINT8;
  mData = new uint8_t;
  *static_cast<uint8_t *>(mData) = uint8_t(value);
}

VariantValue::VariantValue(const double value) {
  mType = VariantType::VARIANT_DOUBLE;
  mData = new double;
  *static_cast<double *>(mData) = double(value);
}

VariantValue::VariantValue(const float value) {
  mType = VariantType::VARIANT_FLOAT;
  mData = new float;
  *static_cast<float *>(mData) = value;
}

VariantValue::VariantValue(const bool value) {
  mType = VariantType::VARIANT_BOOL;
  mData = new bool;
  *static_cast<bool *>(mData) = value;
}

VariantValue::~VariantValue() {
  // TODO ML implement all types. Done.
  switch (mType) {
  case VariantType::VARIANT_FLOAT:
    delete static_cast<float *>(mData);
    break;
  case VariantType::VARIANT_DOUBLE:
    delete static_cast<double *>(mData);
    break;
  case VariantType::VARIANT_INT8:
    delete static_cast<int8_t *>(mData);
    break;
  case VariantType::VARIANT_UINT8:
    delete static_cast<uint8_t *>(mData);
    break;
  case VariantType::VARIANT_INT16:
    delete static_cast<int16_t *>(mData);
    break;
  case VariantType::VARIANT_UINT16:
    delete static_cast<uint16_t *>(mData);
    break;
  case VariantType::VARIANT_INT32:
    delete static_cast<int32_t *>(mData);
    break;
  case VariantType::VARIANT_UINT32:
    delete static_cast<uint32_t *>(mData);
    break;
  case VariantType::VARIANT_INT64:
    delete static_cast<int64_t *>(mData);
    break;
  case VariantType::VARIANT_UINT64:
    delete static_cast<uint64_t *>(mData);
    break;
  case VariantType::VARIANT_STRING:
    delete static_cast<std::string *>(mData);
    break;
  case VariantType::VARIANT_BOOL:
    delete static_cast<bool *>(mData);
    break;
  case VariantType::VARIANT_CHAR:
    delete static_cast<char *>(mData);
    break;
  case VariantType::VARIANT_NONE:
    break;
  }
}

VariantValue::VariantValue(const VariantValue &paramField)
    : mType(paramField.mType) {
  // TODO ML complete all types. Done
  switch (mType) {
  case VariantType::VARIANT_FLOAT:
    mData = new float;
    *static_cast<float *>(mData) = *static_cast<float *>(paramField.mData);
    break;
  case VariantType::VARIANT_DOUBLE:
    mData = new double;
    *static_cast<double *>(mData) = *static_cast<double *>(paramField.mData);
    break;
  case VariantType::VARIANT_STRING:
    mData = new std::string;
    *static_cast<std::string *>(mData) =
        *static_cast<std::string *>(paramField.mData);
    break;
  case VariantType::VARIANT_INT8:
    mData = new int8_t;
    *static_cast<int8_t *>(mData) = *static_cast<int8_t *>(paramField.mData);
    break;
  case VariantType::VARIANT_UINT8:
    mData = new uint8_t;
    *static_cast<uint8_t *>(mData) = *static_cast<uint8_t *>(paramField.mData);
    break;
  case VariantType::VARIANT_INT16:
    mData = new int16_t;
    *static_cast<int16_t *>(mData) = *static_cast<int16_t *>(paramField.mData);
    break;
  case VariantType::VARIANT_UINT16:
    mData = new uint16_t;
    *static_cast<uint16_t *>(mData) =
        *static_cast<uint16_t *>(paramField.mData);
    break;
  case VariantType::VARIANT_INT32:
    mData = new int32_t;
    *static_cast<int32_t *>(mData) = *static_cast<int32_t *>(paramField.mData);
    break;
  case VariantType::VARIANT_UINT32:
    mData = new uint32_t;
    *static_cast<uint32_t *>(mData) =
        *static_cast<uint32_t *>(paramField.mData);
    break;
  case VariantType::VARIANT_INT64:
    mData = new int64_t;
    *static_cast<int64_t *>(mData) = *static_cast<int64_t *>(paramField.mData);
    break;
  case VariantType::VARIANT_UINT64:
    mData = new uint64_t;
    *static_cast<uint64_t *>(mData) =
        *static_cast<uint64_t *>(paramField.mData);
    break;
  case VariantType::VARIANT_CHAR:
    mData = new char;
    *static_cast<char *>(mData) = *static_cast<char *>(paramField.mData);
    break;
  case VariantType::VARIANT_BOOL:
    mData = new bool;
    *static_cast<bool *>(mData) = *static_cast<bool *>(paramField.mData);
    break;
  case VariantType::VARIANT_NONE:
    break;
  default:
    throw std::invalid_argument("Data type not supported");
  }
}

double VariantValue::toDouble() const {
    double v = 0.;
    switch (type()) {
    case VariantType::VARIANT_FLOAT:
        v = get<float>();
        break;
    case VariantType::VARIANT_DOUBLE:
        v = get<double>();
        break;
    case VariantType::VARIANT_STRING:
        v = std::stod(get<std::string>());
        break;
    case VariantType::VARIANT_INT64:
        v = get<int64_t>();
        break;
    case VariantType::VARIANT_INT32:
        v = get<int32_t>();
        break;
    case VariantType::VARIANT_INT16:
        v = get<int16_t>();
        break;
    case VariantType::VARIANT_INT8:
        v = get<int8_t>();
        break;
    case VariantType::VARIANT_UINT64:
        v = get<uint64_t>();
        break;
    case VariantType::VARIANT_UINT32:
        v = get<uint32_t>();
        break;
    case VariantType::VARIANT_UINT16:
        v = get<uint16_t>();
        break;
    case VariantType::VARIANT_UINT8:
        v = get<uint8_t>();
        break;
    case VariantType::VARIANT_NONE:
    case VariantType::VARIANT_BOOL:
    case VariantType::VARIANT_CHAR:
    case VariantType::VARIANT_VARIANT_VECTOR:
    case VariantType::VARIANT_VECTOR_OFFSET:

        std::cerr << __FILE__ << ":" << __LINE__ << " Unsupported value "
                  << std::endl;
        break;
    }
    return v;
}

std::string VariantValue::toString() {
    std::string v;
    switch (type()) {
    case VariantType::VARIANT_FLOAT:
        v = std::to_string(get<float>());
        break;
    case VariantType::VARIANT_DOUBLE:
        v = std::to_string(get<double>());
        break;
    case VariantType::VARIANT_STRING:
        v = get<std::string>();
        break;
    case VariantType::VARIANT_INT64:
        v = std::to_string(get<int64_t>());
        break;
    case VariantType::VARIANT_INT32:
        v = std::to_string(get<int32_t>());
        break;
    case VariantType::VARIANT_INT16:
        v = std::to_string(get<int16_t>());
        break;
    case VariantType::VARIANT_INT8:
        v = std::to_string(get<int8_t>());
        break;
    case VariantType::VARIANT_UINT64:
        v = std::to_string(get<uint64_t>());
        break;
    case VariantType::VARIANT_UINT32:
        v = std::to_string(get<uint32_t>());
        break;
    case VariantType::VARIANT_UINT16:
        v = std::to_string(get<uint16_t>());
        break;
    case VariantType::VARIANT_UINT8:
        v = std::to_string(get<uint8_t>());
        break;
    case VariantType::VARIANT_NONE:
    case VariantType::VARIANT_BOOL:
    case VariantType::VARIANT_CHAR:
    case VariantType::VARIANT_VARIANT_VECTOR:
    case VariantType::VARIANT_VECTOR_OFFSET:

        std::cerr << __FILE__ << ":" << __LINE__ << " Unsupported value "
                  << std::endl;
        break;
    }
    return v;
}
