#include "al/io/al_Toml.hpp"
#include <cstdint>

#include <fstream>
#include <iostream>
#include <sstream>

namespace al {
TomlFile::TomlFile() {}

TomlFile::TomlFile(const std::filesystem::path& path) { open(path); }

bool TomlFile::open(const std::filesystem::path& path)
{
  if (!std::filesystem::exists(path)) {
    std::cout << "[TomlFile] Creating config file: " << path << std::endl;

    std::ofstream configFile{path};
    if (!configFile.is_open()) {
      std::cerr << "[TomlFile] ERROR: Unable to create config file: " << path
                << std::endl;
      return false;
    }

    configFile.close();
  }

  m_path = path;

  try {
    // will throw cpptoml::parse_exception if file is not found
    m_root = cpptoml::parse_file(path.string());
  }
  catch (const cpptoml::parse_exception& err) {
    std::cerr << "[TomlFile] Failed to parse " << path << ": " << err.what()
              << std::endl;
  }
  catch (...) {
    std::cerr << "[TomlFile] Unexpected error" << std::endl;
  }

  return true;
}

bool TomlFile::write(std::filesystem::path path)
{
  if (path.empty()) {
    path = m_path;
  }

  std::ofstream configFile{path, std::ios::trunc};
  if (!configFile.is_open()) {
    std::cerr << "[TomlFile] ERROR: Failed to open file: " << path << std::endl;
    return false;
  }

  std::ostringstream ss;
  ss << (*m_root);
  configFile << ss.str();

  return true;
}

bool TomlFile::hasKey(const std::string& keyName)
{
  if (!m_root) {
    return false;
  }

  return m_root->contains(keyName) || m_root->contains_qualified(keyName);
}
}  // namespace al