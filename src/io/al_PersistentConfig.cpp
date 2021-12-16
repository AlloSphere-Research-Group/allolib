#include "al/io/al_PersistentConfig.hpp"

using namespace al;

void PersistentConfig::setAppName(std::string appName) {
  mAppName = appName;
  //    std::cout << "Persistent config created for " << appName << std::endl;
}

void PersistentConfig::registerDouble(std::string name, double *value) {
  mDoubles.push_back({name, value});
}

void PersistentConfig::registerInt(std::string name, int64_t *value) {
  mInts.push_back({name, value});
}

void PersistentConfig::registerString(std::string name, std::string *value) {
  mStrings.push_back({name, value});
}

void PersistentConfig::registerParameter(Parameter &param) {
  auto fullAddress = param.getFullAddress();
  if (fullAddress.find('.') == std::string::npos) {
    mParameters.push_back(&param);
  } else {
    std::cerr << "ERROR: Parameters with '.' in name are not supported in "
                 "PersistentConfig. Not including."
              << fullAddress << std::endl;
    assert(0 == 1);
  }
}

void PersistentConfig::registerParameter(ParameterInt &param) {
  auto fullAddress = param.getFullAddress();
  if (fullAddress.find('.') == std::string::npos) {
    mParameterInts.push_back(&param);
  } else {
    std::cerr << "ERROR: Parameters with '.' in name are not supported in "
                 "PersistentConfig. Not including."
              << fullAddress << std::endl;
    assert(0 == 1);
  }
}

void PersistentConfig::registerParameter(ParameterString &param) {
  auto fullAddress = param.getFullAddress();
  if (fullAddress.find('.') == std::string::npos) {
    mParameterStrings.push_back(&param);
  } else {
    std::cerr << "ERROR: Parameters with '.' in name are not supported in "
                 "PersistentConfig. Not including."
              << fullAddress << std::endl;
    assert(0 == 1);
  }
}

void PersistentConfig::registerParameter(ParameterColor &param) {
  auto fullAddress = param.getFullAddress();
  if (fullAddress.find('.') == std::string::npos) {
    mParameterColors.push_back(&param);
  } else {
    std::cerr << "ERROR: Parameters with '.' in name are not supported in "
                 "PersistentConfig. Not including: "
              << fullAddress << std::endl;
    assert(0 == 1);
  }
}

bool PersistentConfig::read() {
  if (!File::exists(mAppName + ".toml")) {
    write();
  }
  TomlLoader loader;
  loader.setFile(mAppName + ".toml");
  for (auto &configEntry : mDoubles) {
    if (loader.hasKey<double>(configEntry.first)) {
      *configEntry.second = loader.getd(configEntry.first);
    }
  }
  for (auto &configEntry : mInts) {
    if (loader.hasKey<int64_t>(configEntry.first)) {
      *configEntry.second = loader.geti(configEntry.first);
    }
  }
  for (auto &configEntry : mStrings) {
    if (loader.hasKey<std::string>(configEntry.first)) {
      *configEntry.second = loader.gets(configEntry.first);
    }
  }

  for (auto param : mParameters) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    if (loader.hasKey<std::string>(name)) {
      param->setNoCalls(loader.getd(name));
    }
  }
  for (auto param : mParameterInts) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    if (loader.hasKey<int64_t>(name)) {
      param->setNoCalls(loader.geti(name));
    }
  }
  for (auto param : mParameterStrings) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    if (loader.hasKey<std::string>(name)) {
      param->setNoCalls(loader.gets(name));
    }
  }
  for (auto param : mParameterColors) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    if (loader.root->get_array_of<double>(name)) {
      auto vals = loader.getVector<double>(name);
      param->setNoCalls(Color(vals[0], vals[1], vals[2], vals[3]));
    }
  }
  return true;
}

void PersistentConfig::write() {
  TomlLoader loader;
  loader.setFile(mAppName + ".toml");
  for (auto &configEntry : mDoubles) {
    loader.set(configEntry.first, *configEntry.second);
  }
  for (auto &configEntry : mInts) {
    loader.set(configEntry.first, *configEntry.second);
  }
  for (auto &configEntry : mStrings) {
    loader.set(configEntry.first, *configEntry.second);
  }

  for (auto param : mParameters) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    loader.set(name, param->get());
  }
  for (auto param : mParameterInts) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    loader.set(name, param->get());
  }
  for (auto param : mParameterStrings) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    loader.set(name, param->get());
  }
  for (auto param : mParameterColors) {
    std::string name;
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + ".";
    }
    name += param->getName();
    std::vector<double> rgba;
    rgba.resize(4);
    Color bg = param->get();
    rgba[0] = bg.r;
    rgba[1] = bg.g;
    rgba[2] = bg.b;
    rgba[3] = bg.a;
    loader.setVector<double>(name, rgba);
  }

  loader.writeFile();
}
