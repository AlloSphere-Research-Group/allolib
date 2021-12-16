
#include "al/ui/al_ParameterBundle.hpp"

#include <cstring>
#include <string>

#include "al/protocol/al_OSC.hpp"
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
  if (strcmp(typeid(*parameter).name(), typeid(ParameterBool).name()) ==
      0) { // ParameterBool
    ParameterBool *p = dynamic_cast<ParameterBool *>(parameter);
    p->registerChangeCallback([this, p](float value, ValueSource *src) {
      for (OSCNotifier *n : mNotifiers) {
        n->notifyListeners(bundlePrefix() + p->getFullAddress(), value, src);
      }
    });
  } else if (strcmp(typeid(*parameter).name(), typeid(Parameter).name()) ==
             0) { // Parameter
    //        std::cout << "Register parameter " << parameter->getName() <<
    //        std::endl;
    Parameter *p = dynamic_cast<Parameter *>(parameter);
    p->registerChangeCallback([this, p](float value, ValueSource *src) {
      //            std::cout << "Changed  " << p->getName() << std::endl;
      for (OSCNotifier *n : mNotifiers) {
        n->notifyListeners(bundlePrefix() + p->getFullAddress(), value, src);
      }
    });
  } else if (strcmp(typeid(*parameter).name(),
                    typeid(ParameterString).name()) == 0) { // ParameterString
    //        std::cout << "Register parameter " << parameter->getName() <<
    //        std::endl;
    ParameterString *p = dynamic_cast<ParameterString *>(parameter);
    p->registerChangeCallback([this, p](std::string value, ValueSource *src) {
      //            std::cout << "Changed  " << p->getName() << std::endl;
      for (OSCNotifier *n : mNotifiers) {
        n->notifyListeners(bundlePrefix() + p->getFullAddress(), value, src);
      }
    });
  } else if (strcmp(typeid(*parameter).name(), typeid(ParameterInt).name()) ==
             0) { // Parameter
    //        std::cout << "Register parameter " << parameter->getName() <<
    //        std::endl;
    ParameterInt *p = dynamic_cast<ParameterInt *>(parameter);
    p->registerChangeCallback([this, p](int32_t value, ValueSource *src) {
      //            std::cout << "Changed  " << p->getName() << std::endl;
      for (OSCNotifier *n : mNotifiers) {
        n->notifyListeners(bundlePrefix() + p->getFullAddress(), value, src);
      }
    });
  } else if (strcmp(typeid(*parameter).name(), typeid(ParameterPose).name()) ==
             0) { // ParameterPose
    ParameterPose *p = dynamic_cast<ParameterPose *>(parameter);
    p->registerChangeCallback([this, p](al::Pose value, ValueSource *src) {
      for (OSCNotifier *n : mNotifiers) {
        n->notifyListeners(bundlePrefix() + p->getFullAddress(), value, src);
      }
    });
  } else if (strcmp(typeid(*parameter).name(), typeid(ParameterMenu).name()) ==
             0) { // ParameterMenu
    ParameterMenu *p = dynamic_cast<ParameterMenu *>(parameter);
    p->registerChangeCallback([this, p](int value, ValueSource *src) {
      for (OSCNotifier *n : mNotifiers) {
        n->notifyListeners(bundlePrefix() + p->getFullAddress(), value, src);
      }
    });
  } else if (strcmp(typeid(*parameter).name(),
                    typeid(ParameterChoice).name()) == 0) { // ParameterChoice
    ParameterChoice *p = dynamic_cast<ParameterChoice *>(parameter);
    p->registerChangeCallback([this, p](uint16_t value, ValueSource *src) {
      for (OSCNotifier *n : mNotifiers) {
        n->notifyListeners(bundlePrefix() + p->getFullAddress(), value, src);
      }
    });
  } else if (strcmp(typeid(*parameter).name(), typeid(ParameterVec3).name()) ==
             0) { // ParameterVec3
    ParameterVec3 *p = dynamic_cast<ParameterVec3 *>(parameter);

    p->registerChangeCallback([this, p](al::Vec3f value, ValueSource *src) {
      for (OSCNotifier *n : mNotifiers) {
        n->notifyListeners(bundlePrefix() + p->getFullAddress(), value, src);
      }
    });
  } else if (strcmp(typeid(*parameter).name(), typeid(ParameterVec4).name()) ==
             0) { // ParameterVec4
    ParameterVec4 *p = dynamic_cast<ParameterVec4 *>(parameter);
    p->registerChangeCallback([this, p](al::Vec4f value, ValueSource *src) {
      for (OSCNotifier *n : mNotifiers) {
        n->notifyListeners(bundlePrefix() + p->getFullAddress(), value, src);
      }
    });
  } else if (strcmp(typeid(*parameter).name(), typeid(ParameterColor).name()) ==
             0) { // ParameterColor
    ParameterColor *p = dynamic_cast<ParameterColor *>(parameter);

    p->registerChangeCallback([this, p](Color value, ValueSource *src) {
      Vec4f valueVec(value.r, value.g, value.b, value.a);
      for (OSCNotifier *n : mNotifiers) {
        n->notifyListeners(bundlePrefix() + p->getFullAddress(), valueVec, src);
      }
    });
  } else {
    // TODO this check should be performed on registration
    std::cout << "Unsupported Parameter type for bundle OSC dsitribution"
              << std::endl;
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
