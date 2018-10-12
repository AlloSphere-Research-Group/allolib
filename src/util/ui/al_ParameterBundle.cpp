

#include "al/util/ui/al_ParameterBundle.hpp"

using namespace al;

int ParameterBundle::mBundleCounter = 0;

ParameterBundle::ParameterBundle(std::string name) {
    if (name.size() == 0) {
        mBundleName = "bundle";
    } else {
        mBundleName = name;
    }
    mBundleIndex = mBundleCounter;
    ParameterBundle::mBundleCounter++;
}

std::string ParameterBundle::name() const
{
    return mBundleName;
}

std::string ParameterBundle::bundlePrefix(bool appendCounter) const
{
    std::string prefix = "/" + mBundleName;
    if (appendCounter) {
        prefix += "/" + std::to_string(ParameterBundle::mBundleCounter);
    }
    return prefix;
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
