#ifndef VARIANTVALUE_HPP
#define VARIANTVALUE_HPP

/*
 * Copyright 2021 AlloSphere Research Group
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 *        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * authors: Andres Cabrera
 */

#include <cstdint>
#include <string>

namespace al {

// VariantType enum is equivalent to NC_* types
enum VariantType {
  VARIANT_NONE = 0,
  VARIANT_INT64 = 10,
  VARIANT_INT32 = 4,
  VARIANT_INT16 = 3,
  VARIANT_INT8 = 1,
  VARIANT_UINT64 = 11,
  VARIANT_UINT32 = 9,
  VARIANT_UINT16 = 8,
  VARIANT_UINT8 = 7,
  VARIANT_DOUBLE = 6,
  VARIANT_FLOAT = 5,
  VARIANT_STRING = 12,
  VARIANT_CHAR = 2, // Avoid using VARIANT_CHAR use VARIANT_UINT8
  VARIANT_MAX_ATOMIC_TYPE = VARIANT_STRING,
  VARIANT_BOOL = 99 // Use with care, not an NC type
};

// VariantValue

// FIXME ML complete support for all types
struct VariantValue {
  VariantValue();

  VariantValue(const std::string value);
  VariantValue(const char *value);
  VariantValue(const int64_t value);
  VariantValue(const int32_t value);
  VariantValue(const int8_t value);
  VariantValue(const uint64_t value);
  VariantValue(const uint32_t value);
  VariantValue(const uint8_t value);
  VariantValue(const double value);
  VariantValue(const float value);

  virtual ~VariantValue() {
    // TODO ML implement all types
    switch (mType) {
    case VARIANT_FLOAT:
      delete static_cast<float *>(mData);
      break;
    case VARIANT_DOUBLE:
      delete static_cast<double *>(mData);
      break;
    case VARIANT_STRING:
      delete static_cast<std::string *>(mData);
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
    case VARIANT_NONE:
      break;
    }
  }

  VariantValue(const VariantValue &paramField) : mType(paramField.mType) {
    // TODO ML complete all types
    switch (mType) {
    case VARIANT_FLOAT:
      mData = new float;
      *static_cast<float *>(mData) = *static_cast<float *>(paramField.mData);
      break;
    case VARIANT_STRING:
      mData = new std::string;
      *static_cast<std::string *>(mData) =
          *static_cast<std::string *>(paramField.mData);
      break;
    case VARIANT_INT32:
      mData = new int32_t;
      *static_cast<int32_t *>(mData) =
          *static_cast<int32_t *>(paramField.mData);
      break;
    case VARIANT_UINT32:
      mData = new uint32_t;
      *static_cast<uint32_t *>(mData) =
          *static_cast<uint32_t *>(paramField.mData);
      break;
    case VARIANT_INT64:
      mData = new int64_t;
      *static_cast<int64_t *>(mData) =
          *static_cast<int64_t *>(paramField.mData);
      break;
    case VARIANT_UINT64:
      mData = new uint64_t;
      *static_cast<uint64_t *>(mData) =
          *static_cast<uint64_t *>(paramField.mData);
      break;
    case VARIANT_NONE:
      break;
    default:
      throw std::exception("Data type not supported");
    }
  }

  // Move constructor
  VariantValue(VariantValue &&that) noexcept
      : mType(VARIANT_NONE), mData(nullptr) {
    swap(*this, that);
  }

  // Copy assignment operator
  VariantValue &operator=(const VariantValue &other) {
    VariantValue copy(other);
    swap(*this, copy);
    return *this;
  }

  // Move assignment operator
  VariantValue &operator=(VariantValue &&that) {
    swap(*this, that);
    return *this;
  }

  friend void swap(VariantValue &lhs, VariantValue &rhs) noexcept {
    std::swap(lhs.mData, rhs.mData);
    std::swap(lhs.mType, rhs.mType);
  }

  VariantType type() const { return mType; }

  template <typename type> type get() const {
    return *static_cast<type *>(mData);
  }

  template <typename type> void set(type value) {
    if (std::is_same<type, float>::value) {
      if (mType == VARIANT_FLOAT) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, double>::value) {
      if (mType == VARIANT_FLOAT) {
        *static_cast<float *>(mData) = value;
      }
      if (mType == VARIANT_DOUBLE) {
        *static_cast<double *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, std::string>::value) {
      if (mType == VARIANT_STRING) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, char *>::value) {
      if (mType == VARIANT_STRING) {
        *static_cast<std::string *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, int32_t>::value) {
      if (mType == VARIANT_INT32) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, int>::value) {
      if (mType == VARIANT_INT32) {
        *static_cast<int32_t *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, uint32_t>::value) {
      if (mType == VARIANT_UINT32) {
        *static_cast<uint32_t *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    }
    // TODO add remaining types
  }

protected:
  VariantType mType;
  void *mData;
};
} // namespace al

#endif // VARIANTVALUE_HPP
