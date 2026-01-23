#ifndef AL_SHADER_MANAGER_HPP
#define AL_SHADER_MANAGER_HPP

#include <filesystem>
#include <map>
#include <memory>
#include <string>

#include "al/graphics/al_Shader.hpp"
#include "al/io/al_FileWatcher.hpp"

namespace fs = std::filesystem;
namespace al {

/// @brief contains ShaderProgram, FileWatcher and paths to shader files
struct ManagedShaderProgram {
  std::unique_ptr<ShaderProgram> program;
  std::unique_ptr<FileWatcher> fileWatcher;
  fs::path vertexPath;
  fs::path fragmentPath;
  fs::path geometricPath;
};

/// @brief Class that manages shaders and automatically recompiles on update
class ShaderManager {
 public:
  /// @brief Default constructor. Will add appPaths to the searchPaths
  ShaderManager() : m_shaderDirectory{fs::current_path()} {}

  /// @brief Constructor using a existing SearchPaths
  /// @param paths Existing SearchPaths object
  ShaderManager(const fs::path& dir) : m_shaderDirectory{dir} {}

  /// @brief Set internal SearchPaths to the provided SearchPaths obj
  /// @param paths Desired SearchPaths
  inline void setDirectory(const fs::path& dir) { m_shaderDirectory = dir; }

  /// @brief Set polling intervals for each shader files
  /// @param interval Desired interval in seconds
  void setPollInterval(double interval);

  /// @brief Get reference to the shader program of the name
  /// @param programName name of the shader program
  /// @return Reference to the ShaderProgram
  inline ShaderProgram& get(const std::string& programName)
  {
    return *m_shaders.at(programName).program;
  }

  /// @brief Use the shader program of the name: glUseProgram(ID)
  /// @param programName Name of the shader program
  inline void use(const std::string& programName)
  {
    m_shaders.at(programName).program->use();
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
  void remove(const std::string& programName) { m_shaders.erase(programName); }

  /// @brief Check the shaders for modification and update if needed
  /// @return True if shader has been updated
  bool update();

  /// @brief Print information on currently managed shaders
  void print();

 private:
  /// @brief Load GLSL file and parse the include headers
  /// @param filePath Path to the GLSL file
  /// @return Parsed shader file as string
  std::string loadGLSL(const fs::path& filePath);

  /// @brief Load shader file and compile shader program
  /// @param shaderProgram Obj containing information about shader program
  void compile(ManagedShaderProgram& shaderProgram);

  protected:
  /// @brief Map of shader names to program and info
  std::map<std::string, ManagedShaderProgram> m_shaders;

  /// @brief Directory containing shaders
  fs::path m_shaderDirectory;

  /// @brief Polling interval in seconds
  double m_pollInterval{0};
};

}  // namespace al

#endif