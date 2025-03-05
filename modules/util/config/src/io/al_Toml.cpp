#include "al/io/al_Toml.hpp"

static al::TomlLoader& global_toml_loader() {
  static al::TomlLoader root = []() {
    al::TomlLoader l;
    // try loading default config filename
    try {
      l.setFile("data/al.toml");
    } catch (cpptoml::parse_exception&) {
    }
    return l;
  }();
  return root;
}

void al::TOML::setFile(const std::string& file) {
  global_toml_loader().setFile(file);
}
double al::TOML::getd(const std::string& key) {
  return global_toml_loader().getd(key);
}
int64_t al::TOML::geti(const std::string& key) {
  return global_toml_loader().geti(key);
}
std::string al::TOML::gets(const std::string& key) {
  return global_toml_loader().gets(key);
}
bool al::TOML::getb(const std::string& key) {
  return global_toml_loader().getb(key);
}
double al::TOML::getd(const std::string& table, const std::string& key) {
  return global_toml_loader().getd(table, key);
}
int64_t al::TOML::geti(const std::string& table, const std::string& key) {
  return global_toml_loader().geti(table, key);
}
std::string al::TOML::gets(const std::string& table, const std::string& key) {
  return global_toml_loader().gets(table, key);
}
bool al::TOML::getb(const std::string& table, const std::string& key) {
  return global_toml_loader().getb(table, key);
}
