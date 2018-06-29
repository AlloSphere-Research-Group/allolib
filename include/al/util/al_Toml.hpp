#ifndef INCLUDE_AL_TOML_HPP
#define INCLUDE_AL_TOML_HPP

#include "cpptoml.h"
#include <stdexcept> // std::runtime_error
#include <cstdint>
#include <string>
#include <vector>

namespace al {

struct TomlLoader
{
  std::shared_ptr<cpptoml::table> root;

  TomlLoader() {}

  TomlLoader(const std::string& filename) {
    set_file(filename);
  }

  void set_file(const std::string& filename) {
    // will throw cpptoml::parse_exception if file is not found
    root = cpptoml::parse_file(filename);
  }

  // get top level key's value, supported template parameter type are:
  // std::string, int64_t, double, bool,
  // cpptoml::local_date, cpptoml::local_time, cpptoml::local_datetime,
  // cpptoml::offset_datetime
  template<typename T>
  T get(const std::string& key) {
    if (!root) {
      throw std::runtime_error("al::TomlLoader::get, toml file not set");
    }
    auto val = root->get_as<T>(key);
    if (!val) {
      throw std::runtime_error(std::string("al::TomlLoader::get, key '")
                               + key + std::string("' not found"));
    }
    return *val;
  }

  // get second level (one table below) key's value.
  // give table name and key name
  template<typename T>
  T get(const std::string& table_key, const std::string& key) {
    if (!root) {
      throw std::runtime_error("al::TomlLoader::get, toml file not set");
    }
    auto table = root->get_table(table_key);
    if (!table) {
      throw std::runtime_error(std::string("al::TomlLoader::get, table '")
                               + table_key + std::string("' not found"));
    }
    auto val = table->get_as<T>(key);
    if (!val) {
      throw std::runtime_error(std::string("al::TomlLoader::get, key '")
                               + key + std::string("' not found"));
    }
    return *val;
  }

  double getd(const std::string& key) {
    return get<double>(key);
  }

  int64_t geti(const std::string& key) {
    return get<int64_t>(key);
  }

  std::string gets(const std::string& key) {
    return get<std::string>(key);
  }

 bool getb(const std::string& key) {
    return get<bool>(key);
  }

  double getd(const std::string& table, const std::string& key) {
    return get<double>(table, key);
  }

  int64_t geti(const std::string& table, const std::string& key) {
    return get<int64_t>(table, key);
  }

  std::string gets(const std::string& table, const std::string& key) {
    return get<std::string>(table, key);
  }

 bool getb(const std::string& table, const std::string& key) {
    return get<bool>(table, key);
  }

  template<typename T>
  std::vector<T> getVector(const std::string& key) {
    auto vals = root->get_array_of<T>(key);
    std::vector<T> outVector;
    for (const T &val: *vals) {
      outVector.push_back(val);
    }
    return outVector;
  }

};

namespace TOML {
  void set_file(const std::string& file);
  double getd(const std::string& key);
  int64_t geti(const std::string& key);
  std::string gets(const std::string& key);
  bool getb(const std::string& key);
  double getd(const std::string& table, const std::string& key);
  int64_t geti(const std::string& table, const std::string& key);
  std::string gets(const std::string& table, const std::string& key);
  bool getb(const std::string& table, const std::string& key);
} // namespace TOML

} // namespace al

#endif
