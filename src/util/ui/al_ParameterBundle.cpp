

#include "al/util/ui/al_ParameterBundle.hpp"

using namespace al;

int ParameterBundle::mBundleCounter = 0;

ParameterBundle::ParameterBundle(std::string prefix, bool appendCounter) {
    if (prefix.size() == 0) {
        mBundlePrefix = "bundle";
    } else {
        mBundlePrefix = prefix;
    }
    if (appendCounter) {
        mBundlePrefix += "/" + std::to_string(ParameterBundle::mBundleCounter);
    }
    mBundleIndex = mBundleCounter;
    ParameterBundle::mBundleCounter++;
}

std::string ParameterBundle::bundlePrefix() const
{
    return mBundlePrefix;
}

int ParameterBundle::bundleIndex() const
{
    return mBundleIndex;
}

void ParameterBundle::addParameter(ParameterMeta *parameter) {
    mParameters.push_back(parameter);
}

ParameterBundle &ParameterBundle::operator <<(ParameterMeta *parameter) {
    addParameter(parameter);
    return *this;
}

ParameterBundle &ParameterBundle::operator <<(ParameterMeta &parameter)
{
    addParameter(&parameter);
    return *this;
}
