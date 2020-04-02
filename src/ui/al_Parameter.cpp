
#include "al/ui/al_Parameter.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

#include "al/io/al_File.hpp"

using namespace al;

// Parameter ------------------------------------------------------------------
Parameter::Parameter(std::string parameterName, std::string Group,
                     float defaultValue, std::string prefix, float min,
                     float max)
    : ParameterWrapper<float>(parameterName, Group, defaultValue, prefix, min,
                              max) {
  mFloatValue = defaultValue;
  setDefault(defaultValue);
}

Parameter::Parameter(std::string parameterName, float defaultValue, float min,
                     float max)
    : ParameterWrapper<float>(parameterName, "", defaultValue, "", min, max) {
  mFloatValue = defaultValue;
  setDefault(defaultValue);
}

float Parameter::get() { return mFloatValue; }

void Parameter::setNoCalls(float value, void *blockReceiver) {
  if (value > mMax) value = mMax;
  if (value < mMin) value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value);  //, mProcessUdata);
  }
  if (blockReceiver) {
    runChangeCallbacksSynchronous(value);
  }

  mFloatValue = value;
}

void Parameter::set(float value) {
  if (value > mMax) value = mMax;
  if (value < mMin) value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value);  //, mProcessUdata);
  }

  runChangeCallbacksSynchronous(value);
  mFloatValue = value;
}

// ParameterInt
// ------------------------------------------------------------------
ParameterInt::ParameterInt(std::string parameterName, std::string Group,
                           int32_t defaultValue, std::string prefix,
                           int32_t min, int32_t max)
    : ParameterWrapper<int32_t>(parameterName, Group, defaultValue, prefix, min,
                                max) {
  mIntValue = defaultValue;
  setDefault(defaultValue);
}

int32_t ParameterInt::get() { return mIntValue; }

void ParameterInt::setNoCalls(int32_t value, void *blockReceiver) {
  if (value > mMax) value = mMax;
  if (value < mMin) value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value);  //, mProcessUdata);
  }
  if (blockReceiver) {
    runChangeCallbacksSynchronous(value);
  }

  mIntValue = value;
}

void ParameterInt::set(int32_t value) {
  if (value > mMax) value = mMax;
  if (value < mMin) value = mMin;
  if (mProcessCallback) {
    value = (*mProcessCallback)(value);  //, mProcessUdata);
  }

  runChangeCallbacksSynchronous(value);
  mIntValue = value;
}

// ParameterBool
// ------------------------------------------------------------------
ParameterBool::ParameterBool(std::string parameterName, std::string Group,
                             float defaultValue, std::string prefix, float min,
                             float max)
    : Parameter(parameterName, Group, defaultValue, prefix, min, max) {
  //	mFloatValue = defaultValue;
  setDefault(defaultValue);
}

// --------------------- ParameterMeta ------------

ParameterMeta::ParameterMeta(std::string parameterName, std::string group,
                             std::string prefix)
    : mParameterName(parameterName), mGroup(group), mPrefix(prefix) {
  // TODO: Add better heuristics for slash handling

  using namespace std;

  // remove leading and trailing shashes
  regex re(R"((\w(?:[\w./]*\w)?))");
  auto _parameterName =
      sregex_iterator(parameterName.begin(), parameterName.end(), re);
  auto _group = sregex_iterator(group.begin(), group.end(), re);
  auto _prefix = sregex_iterator(prefix.begin(), prefix.end(), re);
  auto none = sregex_iterator();
  if (_prefix != none) mFullAddress += "/" + _prefix->str();
  if (_group != none) mFullAddress += "/" + _group->str();
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
      0) {  // ParameterBool
    ParameterBool *param = dynamic_cast<ParameterBool *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterBool).name()) == 0);
    dynamic_cast<ParameterBool *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(Parameter).name()) ==
             0) {  // Parameter
    Parameter *param = dynamic_cast<Parameter *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(Parameter).name()) == 0);
    dynamic_cast<Parameter *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterInt).name()) ==
             0) {  // ParameterInt
    ParameterInt *param = dynamic_cast<ParameterInt *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterInt).name()) == 0);
    dynamic_cast<ParameterInt *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterPose).name()) ==
             0) {  // Parameter pose
    ParameterPose *param = dynamic_cast<ParameterPose *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterPose).name()) == 0);
    dynamic_cast<ParameterPose *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterMenu).name()) ==
             0) {  // Parameter
    ParameterMenu *param = dynamic_cast<ParameterMenu *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterMenu).name()) == 0);
    dynamic_cast<ParameterMenu *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterChoice).name()) ==
             0) {  // Parameter
    ParameterChoice *param = dynamic_cast<ParameterChoice *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterChoice).name()) == 0);
    dynamic_cast<ParameterChoice *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterVec3).name()) ==
             0) {  // Parameter
    ParameterVec3 *param = dynamic_cast<ParameterVec3 *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterVec3).name()) == 0);
    dynamic_cast<ParameterVec3 *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterVec4).name()) ==
             0) {  // Parameter
    ParameterVec4 *param = dynamic_cast<ParameterVec4 *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterVec4).name()) == 0);
    dynamic_cast<ParameterVec4 *>(this)->set(param->get());
  } else if (strcmp(typeid(*p).name(), typeid(ParameterColor).name()) ==
             0) {  // Parameter
    ParameterColor *param = dynamic_cast<ParameterColor *>(p);
    // Check that incoming parameter is the same type as this
    assert(strcmp(typeid(*this).name(), typeid(ParameterColor).name()) == 0);
    dynamic_cast<ParameterColor *>(this)->set(param->get());
  } else {
    std::cout << "Unsupported Parameter " << p->getFullAddress() << std::endl;
  }
}
