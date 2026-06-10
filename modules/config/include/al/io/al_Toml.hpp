#ifndef INCLUDE_AL_TOML_HPP
#define INCLUDE_AL_TOML_HPP

#include <cstdint>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

#include "cpptoml.h"

namespace al {
/// @brief A simple wrapper around cpptoml for loading and writing TOML files.
/// Provides some convenience functions for accessing values and error handling.
class TomlFile {
 public:
  /// @brief Constructs an empty TomlFile object. You must call open() before
  /// using it.
  TomlFile();

  /// @brief Constructs a TomlFile object and opens the specified file.
  /// @param path Path to the TOML file to open.
  TomlFile(const std::filesystem::path& path);

  /// @brief Opens the specified TOML file.
  /// @param path Path to the TOML file to open.
  /// @return true if the file was opened successfully, false otherwise.
  bool open(const std::filesystem::path& path);

  /// @brief Writes the current TOML data to the specified file. If no path is
  /// provided, it will overwrite the file that was opened with open().
  bool write(std::filesystem::path path = std::filesystem::path{});

  /// @brief Returns the root table of the TOML file.
  /// @return A shared pointer to the root table.
  std::shared_ptr<cpptoml::table> root() const { return m_root; }

  /// @brief Checks if the specified key exists in the TOML file.
  /// @param keyName The name of the key to check. Can be a top-level key or a
  /// qualified key (e.g., "table.key").
  /// @return true if the key exists, false otherwise.
  bool hasKey(const std::string& keyName);

  /// @brief Gets the value of the specified top level key from the TOML file.
  /// @tparam T The type of the value to get. Supported types include
  /// std::string, int64_t, double, bool, array, table, cpptoml::local_date,
  /// cpptoml::local_time, cpptoml::local_datetime, and
  /// cpptoml::offset_datetime.
  /// @param key The name of the key to get.
  /// @return The value of the key.
  template <typename T>
  T get(const std::string& key)
  {
    if (!m_root) {
      throw std::runtime_error("al::TomlLoader::get, toml file not open");
    }
    auto val = m_root->get_as<T>(key);
    if (!val) {
      throw std::runtime_error(std::string("al::TomlLoader::get, key '") + key +
                               std::string("' not found"));
    }
    return *val;
  }

  /// @brief Gets the value of the specified key from the specified table in the
  /// TOML file.
  /// @tparam T The type of the value to get. Supported types include
  /// std::string, int64_t, double, bool, array, table, cpptoml::local_date,
  /// cpptoml::local_time, cpptoml::local_datetime, and
  /// cpptoml::offset_datetime.
  /// @param table_key The name of the table containing the key.
  /// @param key The name of the key to get.
  /// @return The value of the key.
  template <typename T>
  T get(const std::string& table_key, const std::string& key)
  {
    if (!m_root) {
      throw std::runtime_error("al::TomlLoader::get, toml file not open");
    }
    auto table = m_root->get_table(table_key);
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

  /// @brief Gets a double value from the TOML file.
  inline double getd(const std::string& key) { return get<double>(key); }

  /// @brief Gets an integer value from the TOML file.
  inline int64_t geti(const std::string& key) { return get<int64_t>(key); }

  /// @brief Gets a string value from the TOML file.
  inline std::string gets(const std::string& key)
  {
    return get<std::string>(key);
  }

  /// @brief Gets a boolean value from the TOML file.
  inline bool getb(const std::string& key) { return get<bool>(key); }

  /// @brief Gets a double value from the specified table in the TOML file.
  inline double getd(const std::string& table, const std::string& key)
  {
    return get<double>(table, key);
  }

  /// @brief Gets an integer value from the specified table in the TOML file.
  inline int64_t geti(const std::string& table, const std::string& key)
  {
    return get<int64_t>(table, key);
  }

  /// @brief Gets a string value from the specified table in the TOML file.
  inline std::string gets(const std::string& table, const std::string& key)
  {
    return get<std::string>(table, key);
  }

  /// @brief Gets a boolean value from the specified table in the TOML file.
  inline bool getb(const std::string& table, const std::string& key)
  {
    return get<bool>(table, key);
  }

  /// @brief Gets a vector of values from the TOML file.
  /// @tparam T The type of the values in the vector.
  /// @param key The name of the key to get.
  /// @return The vector of values.
  template <typename T>
  std::vector<T> getVector(const std::string& key)
  {
    if (!m_root) {
      throw std::runtime_error("al::TomlLoader::get, toml file not open");
    }
    auto vals = m_root->get_array_of<T>(key);
    if (!vals) {
      throw std::runtime_error(std::string("al::TomlLoader::get, key '") + key +
                               std::string("' not found or not an array of ") +
                               typeid(T).name());
    }
    return std::move(*vals);
  }

  /// @brief Sets the value of the specified top level key
  /// @tparam T The type of the value to set.
  /// @param key The name of the key to set.
  /// @param value The value to set.
  template <typename T>
  void set(const std::string& key, const T& value)
  {
    if (!m_root) {
      throw std::runtime_error("al::TomlLoader::get, toml file not open");
    }
    m_root->insert(key, value);
  }

  /// @brief Sets a vector of values for the specified top level key
  /// @tparam T The type of the values in the vector.
  /// @param key The name of the key to set.
  /// @param vec The vector of values to set.
  template <typename T>
  void setVector(const std::string& key, const std::vector<T>& vec)
  {
    if (!m_root) {
      throw std::runtime_error("al::TomlLoader::get, toml file not open");
    }
    auto array = cpptoml::make_array();
    for (auto& elem : vec) {
      array->push_back(elem);
    }
    m_root->insert(key, array);
  }

  /// @brief Adds a value to the TOML file if the key is not already present.
  /// @tparam T The type of the value to add.
  /// @param keyName The name of the key to add.
  /// @param value The value to add.
  template <typename T>
  void add_if_absent(const std::string& keyName, const T& value)
  {
    if (!m_root) {
      throw std::runtime_error(
          "al::TomlLoader::add_if_absent, toml file not open");
    }
    auto val = m_root->get_as<T>(keyName);
    if (!val) {
      m_root->insert(keyName, value);
    }
  }

  /// @brief Adds a vector of values to the TOML file if the key is not already
  /// present.
  /// @tparam T The type of the values in the vector.
  /// @param keyName The name of the key to add.
  /// @param value The vector of values to add.
  template <typename T>
  void addVector_if_absent(const std::string& keyName,
                           const std::vector<T>& value)
  {
    if (!m_root) {
      throw std::runtime_error(
          "al::TomlLoader::addVector_if_absent, toml file not open");
    }
    auto val = m_root->get_array_of<T>(keyName);
    if (!val) {
      auto array = cpptoml::make_array();
      for (auto& elem : value) {
        array->push_back(elem);
      }
      m_root->insert(keyName, array);
    }
  }

 private:
  /// @brief The root table of the TOML file.
  std::shared_ptr<cpptoml::table> m_root;
  /// @brief The path to the TOML file.
  std::filesystem::path m_path;
};
}  // namespace al

#endif
