
#include "al/ui/al_Parameter.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

#include "al/io/al_File.hpp"

using namespace al;

// Parameter ------------------------------------------------------------------
Parameter::Parameter(std::string parameterName, std::string group,
                     float defaultValue, float min, float max)
    : ParameterWrapper<float>(parameterName, group, defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

Parameter::Parameter(std::string parameterName, float defaultValue, float min,
                     float max)
    : ParameterWrapper<float>(parameterName, "", defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

Parameter::Parameter(std::string parameterName, std::string Group,
                     float defaultValue, std::string prefix, float min,
                     float max)
    : ParameterWrapper<float>(parameterName, Group, defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

float Parameter::get() { return mValue; }

void Parameter::setNoCalls(float value, void *blockReceiver) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }
  if (blockReceiver) {
    runChangeCallbacksSynchronous(value, nullptr);
  }

  mValue = value;
  mChanged = true;
}

void Parameter::set(float value, ValueSource *src) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }

  runChangeCallbacksSynchronous(value, src);
  mValue = value;
}

// ParameterInt
// ------------------------------------------------------------------
ParameterInt::ParameterInt(std::string parameterName, std::string Group,
                           int32_t defaultValue, int32_t min, int32_t max)
    : ParameterWrapper<int32_t>(parameterName, Group, defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

ParameterInt::ParameterInt(std::string parameterName, std::string Group,
                           int32_t defaultValue, std::string /*prefix*/,
                           int32_t min, int32_t max)
    : ParameterWrapper<int32_t>(parameterName, Group, defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

// int32_t ParameterInt::get() { return mValue; }

void ParameterInt::setNoCalls(int32_t value, void *blockReceiver) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }
  if (blockReceiver) {
    runChangeCallbacksSynchronous(value, nullptr);
  }

  mValue = value;
  mChanged = true;
}

void ParameterInt::set(int32_t value, ValueSource *src) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }

  runChangeCallbacksSynchronous(value, src);
  mValue = value;
}

// ParameterInt8
// ------------------------------------------------------------------
ParameterInt8::ParameterInt8(std::string parameterName, std::string Group,
                             int8_t defaultValue, int8_t min, int8_t max)
    : ParameterWrapper<int8_t>(parameterName, Group, defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

void ParameterInt8::setNoCalls(int8_t value, void *blockReceiver) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }
  if (blockReceiver) {
    runChangeCallbacksSynchronous(value, nullptr);
  }

  mValue = value;
  mChanged = true;
}

void ParameterInt8::set(int8_t value, ValueSource *src) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }

  runChangeCallbacksSynchronous(value, src);
  mValue = value;
}

// ParameterInt16
// ------------------------------------------------------------------
ParameterInt16::ParameterInt16(std::string parameterName, std::string Group,
                               int16_t defaultValue, int16_t min, int16_t max)
    : ParameterWrapper<int16_t>(parameterName, Group, defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

void ParameterInt16::setNoCalls(int16_t value, void *blockReceiver) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }
  if (blockReceiver) {
    runChangeCallbacksSynchronous(value, nullptr);
  }

  mValue = value;
  mChanged = true;
}

void ParameterInt16::set(int16_t value, ValueSource *src) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }

  runChangeCallbacksSynchronous(value, src);
  mValue = value;
}

// ParameterInt64
// ------------------------------------------------------------------
ParameterInt64::ParameterInt64(std::string parameterName, std::string Group,
                               int64_t defaultValue, int64_t min, int64_t max)
    : ParameterWrapper<int64_t>(parameterName, Group, defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

void ParameterInt64::setNoCalls(int64_t value, void *blockReceiver) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }
  if (blockReceiver) {
    runChangeCallbacksSynchronous(value, nullptr);
  }

  mValue = value;
  mChanged = true;
}

void ParameterInt64::set(int64_t value, ValueSource *src) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }

  runChangeCallbacksSynchronous(value, src);
  mValue = value;
}

// ParameterUInt8
// ------------------------------------------------------------------
ParameterUInt8::ParameterUInt8(std::string parameterName, std::string Group,
                               uint8_t defaultValue, uint8_t min, uint8_t max)
    : ParameterWrapper<uint8_t>(parameterName, Group, defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

void ParameterUInt8::setNoCalls(uint8_t value, void *blockReceiver) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }
  if (blockReceiver) {
    runChangeCallbacksSynchronous(value, nullptr);
  }

  mValue = value;
  mChanged = true;
}

void ParameterUInt8::set(uint8_t value, ValueSource *src) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }

  runChangeCallbacksSynchronous(value, src);
  mValue = value;
}

// ParameterUInt16
// ------------------------------------------------------------------
ParameterUInt16::ParameterUInt16(std::string parameterName, std::string Group,
                                 uint16_t defaultValue, uint16_t min,
                                 uint16_t max)
    : ParameterWrapper<uint16_t>(parameterName, Group, defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

void ParameterUInt16::setNoCalls(uint16_t value, void *blockReceiver) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }
  if (blockReceiver) {
    runChangeCallbacksSynchronous(value, nullptr);
  }

  mValue = value;
  mChanged = true;
}

void ParameterUInt16::set(uint16_t value, ValueSource *src) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }

  runChangeCallbacksSynchronous(value, src);
  mValue = value;
}

// ParameterUInt32
// ------------------------------------------------------------------
ParameterUInt32::ParameterUInt32(std::string parameterName, std::string Group,
                                 uint32_t defaultValue, uint32_t min,
                                 uint32_t max)
    : ParameterWrapper<uint32_t>(parameterName, Group, defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

void ParameterUInt32::setNoCalls(uint32_t value, void *blockReceiver) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }
  if (blockReceiver) {
    runChangeCallbacksSynchronous(value, nullptr);
  }

  mValue = value;
  mChanged = true;
}

void ParameterUInt32::set(uint32_t value, ValueSource *src) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }

  runChangeCallbacksSynchronous(value, src);
  mValue = value;
}

// ParameterUInt64
// ------------------------------------------------------------------
ParameterUInt64::ParameterUInt64(std::string parameterName, std::string Group,
                                 uint64_t defaultValue, uint64_t min,
                                 uint64_t max)
    : ParameterWrapper<uint64_t>(parameterName, Group, defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

void ParameterUInt64::setNoCalls(uint64_t value, void *blockReceiver) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }
  if (blockReceiver) {
    runChangeCallbacksSynchronous(value, nullptr);
  }

  mValue = value;
  mChanged = true;
}

void ParameterUInt64::set(uint64_t value, ValueSource *src) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }

  runChangeCallbacksSynchronous(value, src);
  mValue = value;
}

// ParameterDouble
// ------------------------------------------------------------------
ParameterDouble::ParameterDouble(std::string parameterName, std::string Group,
                                 double defaultValue, double min, double max)
    : ParameterWrapper<double>(parameterName, Group, defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

void ParameterDouble::setNoCalls(double value, void *blockReceiver) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }
  if (blockReceiver) {
    runChangeCallbacksSynchronous(value, nullptr);
  }

  mValue = value;
  mChanged = true;
}

void ParameterDouble::set(double value, ValueSource *src) {
  if (value > mMax)
    value = mMax;
  if (value < mMin)
    value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value); //, mProcessUdata);
  }

  runChangeCallbacksSynchronous(value, src);
  mValue = value;
}

// ParameterBool
// ------------------------------------------------------------------
ParameterBool::ParameterBool(std::string parameterName, std::string Group,
                             float defaultValue, float min, float max)
    : Parameter(parameterName, Group, defaultValue, min, max) {
  mValue = defaultValue;
  setDefault(defaultValue);
}

// ParameterBool::ParameterBool(std::string parameterName, std::string Group,
//                             float defaultValue, std::string prefix, float
//                             min, float max)
//    : Parameter(parameterName, Group, defaultValue, prefix, min, max) {
//  mValue = defaultValue;
//  setDefault(defaultValue);
//}

// --------------------- ParameterMeta ------------

ParameterMeta::ParameterMeta(std::string parameterName, std::string group)
    : mParameterName(parameterName), mGroup(group) {
  // TODO: Add better heuristics for slash handling

  using namespace std;

  // remove leading and trailing shashes
  regex re(R"(([^\s](?:[^\s./]*[^\s])?))");
  auto _parameterName =
      sregex_iterator(parameterName.begin(), parameterName.end(), re);
  auto _group = sregex_iterator(group.begin(), group.end(), re);
  auto none = sregex_iterator();
  if (_group != none)
    mFullAddress += "/" + _group->str();
  if (_parameterName != none) {
    mFullAddress += "/" + _parameterName->str();
  } else {
    std::cout << "ParameterMeta ERROR: A valid name must be provided for the "
                 "parameter"
              << std::endl;
    mFullAddress += "/_";
  }

  mDisplayName = mParameterName;
}

void ParameterMeta::set(ParameterMeta *p) {
  // We do a runtime check to determine the type of the parameter to determine
  // how to draw it.
  if (strcmp(typeid(*p).name(), typeid(ParameterBool).name()) ==
      0) { // ParameterBool
    ParameterBool *param = dynamic_cast<ParameterBool *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterBool).name()) == 0);
    dynamic_cast<ParameterBool *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(Parameter).name()) ==
             0) { // Parameter
    Parameter *param = dynamic_cast<Parameter *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(Parameter).name()) == 0);
    dynamic_cast<Parameter *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterInt).name()) ==
             0) { // ParameterInt
    ParameterInt *param = dynamic_cast<ParameterInt *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterInt).name()) == 0);
    dynamic_cast<ParameterInt *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterInt8).name()) ==
             0) { // ParameterInt8
    ParameterInt8 *param = dynamic_cast<ParameterInt8 *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterInt8).name()) == 0);
    dynamic_cast<ParameterInt8 *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterInt16).name()) ==
             0) { // ParameterInt16
    ParameterInt16 *param = dynamic_cast<ParameterInt16 *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterInt16).name()) == 0);
    dynamic_cast<ParameterInt16 *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterInt64).name()) ==
             0) { // ParameterInt64
    ParameterInt64 *param = dynamic_cast<ParameterInt64 *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterInt64).name()) == 0);
    dynamic_cast<ParameterInt64 *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterUInt8).name()) ==
             0) { // ParameterUInt8
    ParameterUInt8 *param = dynamic_cast<ParameterUInt8 *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterUInt8).name()) == 0);
    dynamic_cast<ParameterUInt8 *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterUInt16).name()) ==
             0) { // ParameterUInt16
    ParameterUInt16 *param = dynamic_cast<ParameterUInt16 *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterUInt16).name()) == 0);
    dynamic_cast<ParameterUInt16 *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterUInt32).name()) ==
             0) { // ParameterUInt32
    ParameterUInt32 *param = dynamic_cast<ParameterUInt32 *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterUInt32).name()) == 0);
    dynamic_cast<ParameterUInt32 *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterUInt64).name()) ==
             0) { // ParameterUInt64
    ParameterUInt64 *param = dynamic_cast<ParameterUInt64 *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterUInt64).name()) == 0);
    dynamic_cast<ParameterUInt64 *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterDouble).name()) ==
             0) { // ParameterDouble
    ParameterDouble *param = dynamic_cast<ParameterDouble *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterDouble).name()) == 0);
    dynamic_cast<ParameterDouble *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterPose).name()) ==
             0) { // Parameter pose
    ParameterPose *param = dynamic_cast<ParameterPose *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterPose).name()) == 0);
    dynamic_cast<ParameterPose *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterMenu).name()) ==
             0) { // Parameter
    ParameterMenu *param = dynamic_cast<ParameterMenu *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterMenu).name()) == 0);
    dynamic_cast<ParameterMenu *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterChoice).name()) ==
             0) { // Parameter
    ParameterChoice *param = dynamic_cast<ParameterChoice *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterChoice).name()) == 0);
    dynamic_cast<ParameterChoice *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterVec3).name()) ==
             0) { // Parameter
    ParameterVec3 *param = dynamic_cast<ParameterVec3 *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterVec3).name()) == 0);
    dynamic_cast<ParameterVec3 *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterVec4).name()) ==
             0) { // Parameter
    ParameterVec4 *param = dynamic_cast<ParameterVec4 *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterVec4).name()) == 0);
    dynamic_cast<ParameterVec4 *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterColor).name()) ==
             0) { // Parameter
    ParameterColor *param = dynamic_cast<ParameterColor *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterColor).name()) == 0);
    dynamic_cast<ParameterColor *>(this)->set(param->get());
  } else {
    std::cout << "Unsupported Parameter " << p->getFullAddress() << std::endl;
  }
}
