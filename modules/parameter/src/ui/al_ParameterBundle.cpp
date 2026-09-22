
#include "al/ui/al_ParameterBundle.hpp"
#include <cstdint>

#include <cstring>
#include <string>

#include "al/protocol/al_OSC.hpp"
#include "al/ui/al_ParamCodec.hpp"
#include "al/ui/al_ParameterServer.hpp"

using namespace al;

std::map<std::string, int> ParameterBundle::mBundleCounter =
    std::map<std::string, int>();

ParameterBundle::ParameterBundle(std::string name) {
  if (name.find(" ") != std::string::npos) {
    std::cout << "WARNING: Remove spaces from bundle name." << std::endl;
  }
  if (name.size() == 0) {
    mBundleName = "bundle";
  } else {
    mBundleName = name;
  }
  if (mBundleCounter.find(name) == mBundleCounter.end()) {
    mBundleCounter[name] = 0;
  };
  mBundleIndex = mBundleCounter[name];
  mBundleCounter[name]++;
}

std::string ParameterBundle::name() const { return mBundleName; }

void ParameterBundle::name(std::string newName) { mBundleName = newName; }

std::string ParameterBundle::bundlePrefix() const {
  std::string prefix = mParentPrefix + "/" + mBundleName;
  if (mBundleId.size() == 0) {
    prefix += "/" + std::to_string(mBundleIndex);
  } else {
    prefix += "/" + mBundleId;
  }
  return prefix;
}

int ParameterBundle::bundleIndex() const { return mBundleIndex; }

void ParameterBundle::addParameter(ParameterMeta *parameter) {
  mParameters.push_back(parameter);
  const ParamCodec *codec = paramCodecs().find(*parameter);
  if (codec && codec->attach) {
    codec->attach(*parameter, [this, parameter](ValueSource *src) {
      for (OSCNotifier *n : mNotifiers) {
        n->notifyListeners(bundlePrefix() + parameter->getFullAddress(),
                           parameter, src);
      }
    });
  } else {
    std::cout << "Unsupported Parameter type for bundle OSC distribution: "
              << typeid(*parameter).name() << std::endl;
  }
}

void ParameterBundle::addParameter(ParameterMeta &parameter) {
  addParameter(&parameter);
}

void ParameterBundle::addBundle(ParameterBundle &bundle, std::string id) {
  if (id.size() == 0) {
    id = std::to_string(mBundles.size());
  }
  if (mBundles.find(id) != mBundles.end()) {
    mBundles[id] = std::vector<ParameterBundle *>();
    mBundleIdOrder.push_back(id);
  }
  mBundles[id].push_back(&bundle);
  bundle.mBundleId = id;
  bundle.mParentPrefix = bundlePrefix();
}

ParameterBundle &ParameterBundle::operator<<(ParameterMeta *parameter) {
  addParameter(parameter);
  return *this;
}

ParameterBundle &ParameterBundle::operator<<(ParameterMeta &parameter) {
  addParameter(&parameter);
  return *this;
}

void ParameterBundle::addNotifier(OSCNotifier *notifier) {
  mNotifiers.push_back(notifier);
  for (auto subBundleGroup : bundles()) {
    for (auto *bundle : subBundleGroup.second) {
      bundle->addNotifier(notifier);
    }
  }
}
