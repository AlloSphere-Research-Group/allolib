#ifndef INCLUDE_AL_TOML_HPP
#define INCLUDE_AL_TOML_HPP

#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept> // std::runtime_error
#include <string>
#include <vector>

#include "al/io/al_File.hpp"
#include "cpptoml.h"

namespace al {

/**
@brief TomlLoader class
@ingroup IO
*/
struct TomlLoader {
  std::shared_ptr<cpptoml::table> root;
  std::string mFilename;

  TomlLoader() {}

  TomlLoader(const std::string &filename) { setFile(filename); }

  void setFile(const std::string &filename) {
    if (!al::File::exists(filename)) {
      std::cout << "TomlLoader creating config file: " << filename << std::endl;
      al::File configFile(filename);
      if (configFile.open(filename, "w")) {
        configFile.close();
      } else {
        std::cout << "ERROR: Unable to create config file: " << filename
                  << std::endl;
      }
    }
    // will throw cpptoml::parse_exception if file is not found
    root = cpptoml::parse_file(filename);
    mFilename = filename;
  }

  /// Updates file on disk
  void writeFile(std::string filename = "") {
    if (filename == "") {
      filename = mFilename;
    }
    al::File configFile(filename, "w");
    if (configFile.open()) {
      std::ostringstream ss;
      ss << (*root);
      configFile.write(ss.str());
      configFile.close();
    } else {
      std::cout << "ERROR: Opening toml file '" << filename << "' for writing."
                << std::endl;
    }
  }

  template <typename T>
  void setDefaultValue(const std::string &keyName, const T &value) {
    if (!root) {
      throw std::runtime_error(
          "al::TomlLoader::setDefaultValue, toml file not set");
    }
    auto val = root->get_as<T>(keyName);
    if (!val) {
      root->insert(keyName, value);
    }
  }
  template <typename T>
  void setDefaultValueVector(const std::string &keyName,
                             const std::vector<T> &value) {
    if (!root) {
      throw std::runtime_error(
          "al::TomlLoader::setDefaultValue, toml file not set");
    }
    auto val = root->get_array_of<T>(keyName);
    if (!val) {
      auto array = cpptoml::make_array();
      for (auto &elem : value) {
        array->push_back(elem);
      }
      root->insert(keyName, array);
    }
  }

  template <typename T> bool hasKey(const std::string &keyName) {
    if (root && root->contains(keyName)) {
      auto val = root->get_as<T>(keyName);
      if (val) {
        return true;
      }
    }
    return false;
  }

  template <typename T> void set(const std::string &key, const T &value) {
    if (!root) {
      throw std::runtime_error("al::TomlLoader::get, toml file not set");
    }
    root->insert(key, value);
  }

  // get top level key's value, supported template parameter type are:
  // std::string, int64_t, double, bool,
  // cpptoml::local_date, cpptoml::local_time, cpptoml::local_datetime,
  // cpptoml::offset_datetime
  template <typename T> T get(const std::string &key) {
    if (!root) {
      throw std::runtime_error("al::TomlLoader::get, toml file not set");
    }
    auto val = root->get_as<T>(key);
    if (!val) {
      throw std::runtime_error(std::string("al::TomlLoader::get, key '") + key +
                               std::string("' not found"));
    }
    return *val;
  }

  // get second level (one table below) key's value.
  // give table name and key name
  template <typename T>
  T get(const std::string &table_key, const std::string &key) {
    if (!root) {
      throw std::runtime_error("al::TomlLoader::get, toml file not set");
    }
    auto table = root->get_table(table_key);
    if (!table) {
      throw std::runtime_error(std::string("al::TomlLoader::get, table '") +
                               table_key + std::string("' not found"));
    }
    auto val = table->get_as<T>(key);
    if (!val) {
      throw std::runtime_error(std::string("al::TomlLoader::get, key '") + key +
                               std::string("' not found"));
    }
    return *val;
  }

  double getd(const std::string &key) { return get<double>(key); }

  int64_t geti(const std::string &key) { return get<int64_t>(key); }

  std::string gets(const std::string &key) { return get<std::string>(key); }

  bool getb(const std::string &key) { return get<bool>(key); }

  double getd(const std::string &table, const std::string &key) {
    return get<double>(table, key);
  }

  int64_t geti(const std::string &table, const std::string &key) {
    return get<int64_t>(table, key);
  }

  std::string gets(const std::string &table, const std::string &key) {
    return get<std::string>(table, key);
  }

  bool getb(const std::string &table, const std::string &key) {
    return get<bool>(table, key);
  }

  template <typename T> std::vector<T> getVector(const std::string &key) {
    if (!root) {
      throw std::runtime_error("al::TomlLoader::get, toml file not set");
    }
    auto vals = root->get_array_of<T>(key);
    std::vector<T> outVector;
    for (const T &val : *vals) {
      outVector.push_back(val);
    }
    return outVector;
  }

  template <typename T>
  void setVector(const std::string &key, const std::vector<T> &vec) {
    if (!root) {
      throw std::runtime_error("al::TomlLoader::get, toml file not set");
    }
    auto array = cpptoml::make_array();
    for (auto &elem : vec) {
      array->push_back(elem);
    }
    root->insert(key, array);
  }
};

namespace TOML {
void setFile(const std::string &file);
double getd(const std::string &key);
int64_t geti(const std::string &key);
std::string gets(const std::string &key);
bool getb(const std::string &key);
double getd(const std::string &table, const std::string &key);
int64_t geti(const std::string &table, const std::string &key);
std::string gets(const std::string &table, const std::string &key);
bool getb(const std::string &table, const std::string &key);
} // namespace TOML

} // namespace al

#endif
