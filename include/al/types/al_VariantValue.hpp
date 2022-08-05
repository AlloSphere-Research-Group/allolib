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
#include <iostream>
#include <string>

namespace al {

/*
#define NC_NAT          0
#define NC_BYTE         1
#define NC_CHAR         2
#define NC_SHORT        3
#define NC_INT          4
#define NC_LONG         NC_INT
#define NC_FLOAT        5
#define NC_DOUBLE       6
#define NC_UBYTE        7
#define NC_USHORT       8
#define NC_UINT         9
#define NC_INT64        10
#define NC_UINT64       11
#define NC_STRING       12
*/

// VariantType enum is equivalent to NC_* types
enum class VariantType : int16_t {
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
  VARIANT_BOOL = 99,           // Use with care, not an NC type
  VARIANT_VECTOR_OFFSET = 100, // Vectors will have this value added to the type
  VARIANT_VARIANT_VECTOR = 200 // Defines a vector of variants
};

// VariantValue

// FIXME ML complete support for all types. done.
struct VariantValue {
  VariantValue();

  VariantValue(const std::string value);
  VariantValue(const char *value);
  VariantValue(const int64_t value);
  VariantValue(const int32_t value);
  VariantValue(const int16_t value);
  VariantValue(const int8_t value);
  VariantValue(const uint64_t value);
  VariantValue(const uint32_t value);
  VariantValue(const uint16_t value);
  VariantValue(const uint8_t value);
  VariantValue(const double value);
  VariantValue(const float value);
  VariantValue(const bool value);

  virtual ~VariantValue();

  VariantValue(const VariantValue &paramField);

  // Move constructor
  VariantValue(VariantValue &&that) noexcept
      : mType(VariantType::VARIANT_NONE), mData(nullptr) {
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

  bool operator==(const VariantValue &other) {
    return this->toDouble() == other.toDouble();
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
      if (mType == VariantType::VARIANT_FLOAT) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, double>::value) {
      if (mType == VariantType::VARIANT_DOUBLE) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, std::string>::value) {
      if (mType == VariantType::VARIANT_STRING) {
        *static_cast<type *>(mData) = value;
      } else if (mType == VariantType::VARIANT_MAX_ATOMIC_TYPE) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, char *>::value) {
      if (mType == VariantType::VARIANT_CHAR) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, int8_t>::value) {
      if (mType == VariantType::VARIANT_INT8) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, int16_t>::value) {
      if (mType == VariantType::VARIANT_INT16) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, int32_t>::value) {
      if (mType == VariantType::VARIANT_INT32) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, int64_t>::value) {
      if (mType == VariantType::VARIANT_INT64) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, uint8_t>::value) {
      if (mType == VariantType::VARIANT_UINT8) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, uint16_t>::value) {
      if (mType == VariantType::VARIANT_UINT16) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, uint32_t>::value) {
      if (mType == VariantType::VARIANT_UINT32) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, uint64_t>::value) {
      if (mType == VariantType::VARIANT_UINT64) {
        *static_cast<type *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    } else if (std::is_same<type, bool>::value) {
      if (mType == VariantType::VARIANT_BOOL) {
        *static_cast<bool *>(mData) = value;
      } else {
        std::cerr
            << "ERROR: Unexpected type for parameter field set(). Ignoring."
            << std::endl;
      }
    }
    // TODO ML add remaining types
  }

  /**
   * @brief Convert any type to double
   * @return value as a double
   */
  double toDouble() const;

  /**
   * @brief Convert any type to string
   * @return value as a string
   */
  std::string toString();

protected:
  VariantType mType;
  void *mData;
};
} // namespace al

#endif // VARIANTVALUE_HPP
