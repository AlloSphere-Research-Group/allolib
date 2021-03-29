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
  loader.writeFile();
}
