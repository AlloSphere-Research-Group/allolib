#ifndef AL_DISCRETEPARAMETERVALUES_HPP
#define AL_DISCRETEPARAMETERVALUES_HPP

#include <iostream>
#include <mutex>
#include <string>
#include <vector>

#include "al/types/al_VariantValue.hpp"

namespace al {

class DiscreteParameterValues {
public:
  //  // TODO add support for in16 and uint16
  //  typedef enum {
  //    BOOL,
  //    FLOAT,
  //    DOUBLE,
  //    INT8,
  //    UINT8,
  //    INT32,
  //    UINT32,
  //    INT64,
  //    UINT64,
  //    STRING
  //  } Datatype;

  DiscreteParameterValues(VariantType datatype = VariantType::VARIANT_FLOAT);

  ~DiscreteParameterValues();

  VariantType getDataType() { return mDatatype; }

  size_t size();
  //  void reserve(size_t totalSize);

  void sort();
  void clear();

  template <typename SpaceDataType>
  void append(std::vector<SpaceDataType> &values, std::string idprefix = "") {
    // TODO validate data type
    append(values.data(), values.size(), idprefix);
  }

  void append(void *values, size_t count, std::string idprefix = "");

  // Set limits from internal data
  //  void conform();

  // Query ----

  /**
   * @brief getFirstIndexForId
   * @param id
   * @param reverse
   * @return
   *
   * Returns numeric_limits<size_t>::max() if id is not found.
   */
  size_t getFirstIndexForId(std::string id, bool reverse = false);

  template <typename SpaceDataType>
  std::vector<std::string> getIdsForValue(SpaceDataType value);

  float at(size_t x);

  std::string idAt(size_t x);

  /**
   * @brief getIndexForValue
   * @param value
   * @return the index of the value
   *
   * Approximates to the closest extant value, except beyond minimum and maximum
   * values. Returns SIZE_MAX if value outside the range.
   */
  // TODO add generic size_t getIndexForValue(SpaceDataType value)
  size_t getIndexForValue(float value);

  template <typename SpaceDataType>
  std::vector<size_t> getIndecesForValue(SpaceDataType value);

  // Access to complete sets
  template <typename VecDataType> std::vector<VecDataType> getValues() {
    // TODO validate sizes
    return std::vector<VecDataType>((VecDataType *)mValues,
                                    (VecDataType *)mValues + size());
  }
  void *getValuesPtr() { return mValues; }

  void setIds(std::vector<std::string> newIds) { mIds = newIds; }
  std::vector<std::string> getIds();

  size_t stride();

  // Protect parameter space (to avoid access during modification)
  // TODO all readers above need to use this lock
  void lock() { mLock.lock(); }
  void unlock() { mLock.unlock(); }

protected:
  template <typename VecDataType> std::vector<VecDataType> getAsVector() {
    std::vector<VecDataType> vec;
    VecDataType *castValues = static_cast<VecDataType>(mValues);
    vec.assign(castValues, castValues + mSize);
    return vec;
  }
  std::string valueToString(void *value);
  double valueToFloat(void *value);
  int64_t valueToInt64(void *value);

private:
  VariantType mDatatype{VARIANT_FLOAT};
  size_t mSize{0};
  uint8_t mDataSize;
  unsigned char *mValues{nullptr};
  std::vector<std::string> mIds;

  std::mutex mLock;
};

template <typename SpaceDataType>
std::vector<size_t>
DiscreteParameterValues::getIndecesForValue(SpaceDataType value) {
  std::vector<size_t> indeces;
  SpaceDataType *castValues = (SpaceDataType *)mValues;
  for (size_t i = 0; i < mSize; i++) {
    if (castValues[i] == value) {
      indeces.push_back(i);
    }
  }
  return indeces;
}

template <typename SpaceDataType>
std::vector<std::string>
DiscreteParameterValues::getIdsForValue(SpaceDataType value) {
  std::vector<std::string> ids;
  auto indeces = getIndecesForValue(value);
  for (auto index : indeces) {
    if (index < mIds.size()) {
      ids.push_back(mIds[index]);
    } else {
      std::cerr << __FUNCTION__ << " Invalid index to id" << std::endl;
    }
  }
  return ids;
}

} // namespace al

#endif // AL_DISCRETEPARAMETERVALUES_HPP
