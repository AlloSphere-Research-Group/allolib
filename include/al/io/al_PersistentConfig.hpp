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
  /**
   * @brief Set name of app. This determines the file name for the config file.
   * @param appName
   */
  void setAppName(std::string appName);

  void registerDouble(std::string name, double *value);

  void registerInt(std::string name, int64_t *value);

  void registerString(std::string name, std::string *value);

  bool read();

  void write();

 private:
  std::string mAppName{"app"};
  std::vector<std::pair<std::string, double *>> mDoubles;
  std::vector<std::pair<std::string, int64_t *>> mInts;
  std::vector<std::pair<std::string, std::string *>> mStrings;
};

}  // namespace al

#endif  // INCLUDE_PERSISTENTCONFIG_HPP
