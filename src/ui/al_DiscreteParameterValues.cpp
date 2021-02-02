#include "al/ui/al_DiscreteParameterValues.hpp"

#include <iostream>
#include <algorithm>
#include <cassert>

#include <cstring>

using namespace al;

DiscreteParameterValues::DiscreteParameterValues(
    DiscreteParameterValues::Datatype datatype)
    : mDatatype(datatype) {
  switch (datatype) {
  case FLOAT:
    mDataSize = 4;
    break;
  case DOUBLE:
    mDataSize = 8;
    break;
  case INT8:
  case UINT8:
    mDataSize = 1;
    break;
  case INT32:
  case UINT32:
    mDataSize = 4;
    break;
  case INT64:
  case UINT64:
    mDataSize = 8;
    break;
  }
}

DiscreteParameterValues::~DiscreteParameterValues() {
  lock();
  if (mValues) {
    free(mValues);
  }
  unlock();
}

size_t DiscreteParameterValues::size() { return mSize; }

// void DiscreteParameterValues::reserve(size_t totalSize) {
//  mValues.reserve(totalSize);
//}

void DiscreteParameterValues::sort() {
  lock();

  std::vector<size_t> indeces;
  indeces.resize(size());
  for (size_t i = 0; i < indeces.size(); i++) {
    indeces[i] = i;
  }
  assert(0 == 1);
  // FIXME implement
  //  auto currentValues = getAsVector<double>();
  //  std::sort(indeces.begin(), indeces.end(), sort_indices(mValues.data()));
  //  std::vector<float> newValues;
  //  std::vector<std::string> newIds;
  //  newValues.reserve(indeces.size());
  //  newIds.reserve(indeces.size());
  //  if (size() != mIds.size() && mIds.size() > 0) {
  //    std::cerr << "ERROR: sort() will crash (or lead ot unexpected behavior)
  //    as "
  //                 "the size of values and ids don't match."
  //              << std::endl;
  //  }
  //  for (size_t i = 0; i < indeces.size(); i++) {
  //    size_t index = indeces[i];
  //    newValues.push_back(mValues[index]);
  //    if (mIds.size() > 0) {
  //      newIds.push_back(mIds[index]);
  //    }
  //  }
  //  mValues = newValues;
  //  mIds = newIds;
  //  unlock();
}

void DiscreteParameterValues::clear() {
  lock();
  mSize = 0;
  free(mValues);
  mValues = nullptr;
  mIds.clear();
  unlock();
}

// void DiscreteParameterValues::push_back(float value, std::string id) {

//  // FIXME There is no check to see if value is already present. Could cause
//  // trouble
//  // if value is there already.
//  mValues.emplace_back(value);

//  if (id.size() > 0) {
//    mIds.push_back(id);
//    if (mIds.size() != mValues.size()) {
//      std::cerr
//          << " ERROR! value and id mismatch in parameter space! This is bad."
//          << std::endl;
//    }
//  }

//  if (value > mParameterValue.max()) {
//    mParameterValue.max(value);
//  }
//  if (value < mParameterValue.min()) {
//    mParameterValue.min(value);
//  }

//  onDimensionMetadataChange(this);
//}

void DiscreteParameterValues::append(void *values, size_t count,
                                     std::string idprefix) {
  size_t oldSize = size();
  if (mValues) {
    void *oldValues = mValues;
    mValues = (unsigned char *)malloc((oldSize + count) * mDataSize);
    mSize = oldSize + count;
    memcpy(mValues, oldValues, oldSize * mDataSize);
    free(oldValues);
  } else {
    mValues = (unsigned char *)malloc(count * mDataSize);
    mSize = count;
  }

  memcpy(mValues + (oldSize * mDataSize), values, count * mDataSize);
  bool useIds = false;
  if (mIds.size() > 0 || idprefix.size() > 0) {
    useIds = true;
    mIds.resize(size());
  }
  auto idIt = mIds.begin() + oldSize;
  auto valuesPtr = mValues + oldSize;
  if (useIds) {
    for (size_t i = 0; i < count; i++) {
      *idIt = idprefix + valueToString(valuesPtr);
      idIt++;
      valuesPtr += mDataSize;
    }
  }
}

size_t DiscreteParameterValues::getFirstIndexForId(std::string id,
                                                   bool reverse) {
  size_t paramIndex = std::numeric_limits<size_t>::max();

  if (!reverse) {
    size_t i = 0;
    for (auto it = mIds.begin(); it != mIds.end(); it++) {
      if (*it == id) {
        paramIndex = i;
        break;
      }
      i++;
    }
  } else {
    size_t i = mIds.size() - 1;
    for (auto it = mIds.rbegin(); it != mIds.rend(); it++) {
      if (*it == id) {
        paramIndex = i;
        break;
      }
      i--;
    }
  }
  return paramIndex;
}

float DiscreteParameterValues::at(size_t x) {
  if (x < size()) {
    return valueToFloat(mValues + (x * mDataSize));
  }
  return 0.0f;
}

std::string DiscreteParameterValues::idAt(size_t x) {
  if (x < mIds.size()) {
    return mIds[x];
  } else if (mIds.size() == 0 && x < size()) {
    return std::to_string(at(x));
  } else {
    return std::string();
  }
}

size_t DiscreteParameterValues::getIndexForValue(float value) {

  size_t paramIndex = SIZE_MAX;
  if (size() == 0) {
    return SIZE_MAX;
  } else if (size() == 1) {
    if (value == valueToFloat(mValues)) {
      return 0;
    }
  }
  for (size_t it = 0; it < size() - 1; it++) {
    auto v0 = valueToFloat(&(mValues[it * mDataSize]));
    auto v1 = valueToFloat(&(mValues[(it + 1) * mDataSize]));
    if (v0 < v1) {
      if (value >= v0 && value <= ((v0 + v1) * 0.5)) {
        paramIndex = it;
        break;
      } else if (value > ((v0 + v1) * 0.5) && value <= v1) {
        paramIndex = it + 1;
        break;
      }
    } else {
      // descending space
      if (value <= v0 && value >= ((v0 + v1) * 0.5)) {
        paramIndex = it;
        break;
      } else if (value < ((v0 + v1) * 0.5) && value >= v1) {
        paramIndex = it + 1;
        break;
      }
    }
  }
  return paramIndex;
}

std::vector<std::string> DiscreteParameterValues::getIds() { return mIds; }

std::string DiscreteParameterValues::valueToString(void *value) {
  std::string valueStr;
  switch (mDatatype) {
  case FLOAT: {
    float val = *(static_cast<float *>(value));
    valueStr = std::to_string(val);
  } break;
  case DOUBLE: {
    double val = *(static_cast<double *>(value));
    valueStr = std::to_string(val);
  } break;
  case INT8: {
    int8_t val = *(static_cast<int8_t *>(value));
    valueStr = std::to_string(val);
  } break;
  case UINT8: {
    uint8_t val = *(static_cast<uint8_t *>(value));
    valueStr = std::to_string(val);
  } break;
  case INT32: {
    int32_t val = *(static_cast<int32_t *>(value));
    valueStr = std::to_string(val);
  } break;
  case UINT32: {
    uint32_t val = *(static_cast<uint32_t *>(value));
    valueStr = std::to_string(val);
  } break;
  case INT64: {
    int64_t val = *(static_cast<int64_t *>(value));
    valueStr = std::to_string(val);
  } break;
  case UINT64: {
    uint64_t val = *(static_cast<uint64_t *>(value));
    valueStr = std::to_string(val);
  } break;
    //  case STRING: {
    //    std::string val = (static_cast<char *>(value));
    //    valueStr = val;
    //  } break;
  }
  return valueStr;
}

double DiscreteParameterValues::valueToFloat(void *value) {
  double valueDbl;
  switch (mDatatype) {
  case FLOAT: {
    valueDbl = *(static_cast<float *>(value));
  } break;
  case DOUBLE: {
    valueDbl = *(static_cast<double *>(value));
  } break;
  case INT8: {
    valueDbl = *(static_cast<int8_t *>(value));
  } break;
  case UINT8: {
    valueDbl = *(static_cast<uint8_t *>(value));
  } break;
  case INT32: {
    valueDbl = *(static_cast<int32_t *>(value));
  } break;
  case UINT32: {
    valueDbl = *(static_cast<uint32_t *>(value));
  } break;
  case INT64: {
    valueDbl = *(static_cast<int64_t *>(value));
  } break;
  case UINT64: {
    valueDbl = *(static_cast<uint64_t *>(value));
  } break;
  }
  return valueDbl;
}

int64_t DiscreteParameterValues::valueToInt64(void *value) {
  int64_t valueInt;
  switch (mDatatype) {
  case FLOAT: {
    valueInt = *(static_cast<float *>(value));
  } break;
  case DOUBLE: {
    valueInt = *(static_cast<double *>(value));
  } break;
  case INT8: {
    valueInt = *(static_cast<int8_t *>(value));
  } break;
  case UINT8: {
    valueInt = *(static_cast<uint8_t *>(value));
  } break;
  case INT32: {
    valueInt = *(static_cast<int32_t *>(value));
  } break;
  case UINT32: {
    valueInt = *(static_cast<uint32_t *>(value));
  } break;
  case INT64: {
    valueInt = *(static_cast<int64_t *>(value));
  } break;
  case UINT64: {
    valueInt = *(static_cast<uint64_t *>(value));
  } break;
  }
  return valueInt;
}
