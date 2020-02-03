#ifndef INCLUDE_PERSISTENTCONFIG_HPP
#define INCLUDE_PERSISTENTCONFIG_HPP

#include <cinttypes>
#include <iostream>
#include <string>
#include <vector>

#include "al/io/al_File.hpp"
#include "al/io/al_Toml.hpp"

namespace al {

/**
 * @brief The PersistentConfig class allows easy sotrage of persisitent values
 */
class PersistentConfig {
 public:
  void setAppName(std::string appName) {
    mAppName = appName;
    //    std::cout << "Persistent config created for " << appName << std::endl;
  }

  void registerDouble(std::string name, double *value) {
    mDoubles.push_back({name, value});
  }

  void registerInt(std::string name, int64_t *value) {
    mInts.push_back({name, value});
  }

  void registerString(std::string name, std::string *value) {
    mStrings.push_back({name, value});
  }

  bool read() {
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

  void write() {
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

 private:
  std::string mAppName{"app"};
  std::vector<std::pair<std::string, double *>> mDoubles;
  std::vector<std::pair<std::string, int64_t *>> mInts;
  std::vector<std::pair<std::string, std::string *>> mStrings;
};

}  // namespace al

#endif  // INCLUDE_PERSISTENTCONFIG_HPP
