
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

#include "al/util/ui/al_Parameter.hpp"
#include "al/core/io/al_File.hpp"

using namespace al;

// Parameter ------------------------------------------------------------------
Parameter::Parameter(std::string parameterName, std::string Group,
                     float defaultValue,
                     std::string prefix,
                     float min,
                     float max) :
    ParameterWrapper<float>(parameterName, Group, defaultValue, prefix, min, max)
{
	mFloatValue = defaultValue;
}

float Parameter::get()
{
	return mFloatValue;
}

void Parameter::setNoCalls(float value, void *blockReceiver)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
	if (mProcessCallback) {
        value = (*mProcessCallback)(value); //, mProcessUdata);
	}
    if (blockReceiver) {
        for(auto cb:mCallbacks) {
            (*cb)(value);
        }
	}

	mFloatValue = value;
}

void Parameter::set(float value)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
    if (mProcessCallback) {
        value = (*mProcessCallback)(value); //, mProcessUdata);
    }
    for(auto cb:mCallbacks) {
        (*cb)(value);
    }
    mFloatValue = value;
}

// ParameterInt ------------------------------------------------------------------
ParameterInt::ParameterInt(std::string parameterName, std::string Group,
                     int32_t defaultValue,
                     std::string prefix,
                     int32_t min,
                     int32_t max) :
    ParameterWrapper<int32_t>(parameterName, Group, defaultValue, prefix, min, max)
{
	mIntValue = defaultValue;
}

int32_t ParameterInt::get()
{
	return mIntValue;
}

void ParameterInt::setNoCalls(int32_t value, void *blockReceiver)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
	if (mProcessCallback) {
        value = (*mProcessCallback)(value); //, mProcessUdata);
	}
    if (blockReceiver) {
        for(auto cb:mCallbacks) {
            (*cb)(value);
        }
	}

	mIntValue = value;
}

void ParameterInt::set(int32_t value)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
    if (mProcessCallback) {
        value = (*mProcessCallback)(value); //, mProcessUdata);
    }
    for(auto cb:mCallbacks) {
        (*cb)(value);
    }
    mIntValue = value;
}

// ParameterBool ------------------------------------------------------------------
ParameterBool::ParameterBool(std::string parameterName, std::string Group,
                     float defaultValue,
                     std::string prefix,
                     float min,
                     float max) :
    Parameter(parameterName, Group, defaultValue, prefix, min, max)
{
//	mFloatValue = defaultValue;
}


//void ParameterBool::setNoCalls(float value, void *blockReceiver)
//{
//	if (value > mMax) value = mMax;
//	if (value < mMin) value = mMin;
//	if (mProcessCallback) {
//		value = mProcessCallback(value, mProcessUdata);
//	}
//	if (blockReceiver) {
//		for(size_t i = 0; i < mCallbacks.size(); ++i) {
//			if (mCallbacks[i]) {
//				mCallbacks[i](value, this, mCallbackUdata[i], blockReceiver);
//			}
//		}
//	}

//	mFloatValue = value;
//}

//void ParameterBool::set(float value)
//{
//	if (value > mMax) value = mMax;
//	if (value < mMin) value = mMin;
//	if (mProcessCallback) {
//		value = mProcessCallback(value, mProcessUdata);
//	}
//	mFloatValue = value;
//	for(size_t i = 0; i < mCallbacks.size(); ++i) {
//		if (mCallbacks[i]) {
//			mCallbacks[i](value, this, mCallbackUdata[i], NULL);
//		}
//	}
//}


// --------------------- ParameterMeta ------------

ParameterMeta::ParameterMeta(std::string parameterName, std::string group, std::string prefix) :
    mParameterName(parameterName), mGroup(group), mPrefix(prefix)
{
    //TODO: Add better heuristics for slash handling

  using namespace std;

  // remove leading and trailing shashes
  regex re(R"((\w(?:[\w/]*\w)?))");
  auto _parameterName =
      sregex_iterator(parameterName.begin(), parameterName.end(), re);
  auto _group = sregex_iterator(group.begin(), group.end(), re);
  auto _prefix = sregex_iterator(prefix.begin(), prefix.end(), re);
  auto none = sregex_iterator();
  if (_prefix != none) mFullAddress += "/" + _prefix->str();
  if (_group != none) mFullAddress += "/" + _group->str();
  assert(_parameterName != none);
  mFullAddress += "/" + _parameterName->str();

    mDisplayName = mParameterName;
}
