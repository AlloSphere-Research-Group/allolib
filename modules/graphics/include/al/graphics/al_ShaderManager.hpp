#ifndef AL_SHADER_MANAGER_HPP
#define AL_SHADER_MANAGER_HPP

#include <map>
#include <memory>
#include <string>

#include "al/graphics/al_Shader.hpp"
#include "al/io/al_File.hpp"
#include "al/io/al_FileWatcher.hpp"

namespace al {

/// @brief contains ShaderProgram, FileWatcher and paths to shader files
struct ManagedShaderProgram {
  std::unique_ptr<ShaderProgram> program;
  std::unique_ptr<FileWatcher> fileWatcher;
  std::string vertexShaderPath;
  std::string fragmentShaderPath;
  std::string geometricShaderPath;
};

/// @brief Class that manages shaders and automatically recompiles on update
class ShaderManager {
 public:
  /// @brief Default constructor. Will add appPaths to the searchPaths
  ShaderManager() { searchPaths.addAppPaths(); }

  /// @brief Constructor using a existing SearchPaths
  /// @param paths Existing SearchPaths object
  ShaderManager(const SearchPaths& paths) : searchPaths(paths) {}

  /// @brief Set internal SearchPaths to the provided SearchPaths obj
  /// @param paths Desired SearchPaths
  inline void setSearchPaths(const SearchPaths& paths) { searchPaths = paths; }

  /// @brief Set polling intervals for each shader files
  /// @param interval Desired interval in seconds
  void setPollInterval(const al_sec& interval);

  /// @brief Get reference to the shader program of the name
  /// @param programName name of the shader program
  /// @return Reference to the ShaderProgram
  inline ShaderProgram& get(const std::string& programName)
  {
    return *shaders.at(programName).program;
  }

  /// @brief Use the shader program of the name: glUseProgram(ID)
  /// @param programName Name of the shader program
  inline void use(const std::string& programName)
  {
    shaders.at(programName).program->use();
  }

  /// @brief Add new shader program to the shader manager
  /// @param programName Name of the shader program
  /// @param vertexShaderName Filename of vertex shader
  /// @param fragmentShaderName Filename of fragment shader
  /// @param geometricShaderName Filename of geometric shader if it exists
  void add(const std::string& programName, const std::string& vertexShaderName,
           const std::string& fragmentShaderName,
           const std::string& geometricShaderName = "");

  /// @brief Removes shader program from the manager
  /// @param programName Name of the shader program
  void remove(const std::string& programName) { shaders.erase(programName); }

  /// @brief Check the shaders for modification and update if needed
  /// @return True if shader has been updated
  bool update();

  /// @brief Print information on currently managed shaders
  void print();

 private:
  /// @brief Load GLSL file and parse the include headers
  /// @param filePath Path to the GLSL file
  /// @return Parsed shader file as string
  std::string loadGLSL(const std::string& filePath);

  /// @brief Load shader file and compile shader program
  /// @param shaderProgram Obj containing information about shader program
  void compile(ManagedShaderProgram& shaderProgram);

  /// @brief Map of shader names to program and info
  std::map<std::string, ManagedShaderProgram> shaders;

  /// @brief Internal searchPath object for handling includes
  SearchPaths searchPaths;

  /// @brief Polling interval in seconds
  al_sec pollInterval{0};
};

}  // namespace al

#endif