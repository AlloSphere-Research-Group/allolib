#include "al/graphics/al_ShaderManager.hpp"

namespace al {

void ShaderManager::setPollInterval(const al_sec& interval)
{
  pollInterval = interval;

  for (auto& shader : shaders) {
    shader.second.fileWatcher->setPollInterval(interval);
  }
}

std::string ShaderManager::loadGLSL(const std::string& filePath)
{
  std::string code = File::read(filePath);
  std::string include_code{"#include \""};
  size_t from = code.find(include_code);
  if (from != std::string::npos) {
    size_t capture = from + include_code.size();
    size_t to = code.find("\"", capture);
    std::string include_filename = code.substr(capture, to - capture);
    std::string replacement =
        File::read(searchPaths.find(include_filename).filepath());
    code = code.replace(from, to - from + 2, replacement);
    // printf("code: %s\n", code.data());
  }
  return code;
}

void ShaderManager::compile(ManagedShaderProgram& shaderProgram)
{
  std::string vertexShader = loadGLSL(shaderProgram.vertexShaderPath);
  std::string fragmentShader = loadGLSL(shaderProgram.fragmentShaderPath);
  std::string geometricShader = loadGLSL(shaderProgram.geometricShaderPath);

  shaderProgram.program->compile(vertexShader, fragmentShader, geometricShader);
}

void ShaderManager::add(const std::string& programName,
                        const std::string& vertexShaderName,
                        const std::string& fragmentShaderName,
                        const std::string& geometricShaderName)
{
  auto search = shaders.find(programName);
   if (search != shaders.end())
  {
    shaders[programName].program->destroy();
    std::cerr << "Shader Program with same name already exists" << std::endl;
  }

  shaders[programName] = {std::make_unique<ShaderProgram>(),
                          std::make_unique<FileWatcher>(),
                          searchPaths.find(vertexShaderName).filepath(),
                          searchPaths.find(fragmentShaderName).filepath(),
                          searchPaths.find(geometricShaderName).filepath()};

  auto& shaderProgram = shaders[programName];

  shaderProgram.fileWatcher->watch(shaderProgram.vertexShaderPath);
  shaderProgram.fileWatcher->watch(shaderProgram.fragmentShaderPath);
  if (!shaderProgram.geometricShaderPath.empty()) {
    shaderProgram.fileWatcher->watch(shaderProgram.geometricShaderPath);
  }

  shaderProgram.fileWatcher->setPollInterval(pollInterval);

  compile(shaderProgram);
}

bool ShaderManager::update()
{
  bool updated = false;
  for (auto& shader : shaders) {
    if (shader.second.fileWatcher->poll()) {
      compile(shader.second);
      std::cout << "Updated shader: " << shader.first << std::endl;
      updated = true;
    }
  }
  return updated;
}

void ShaderManager::print()
{
  std::cout << "[ShaderManager] Managed shader list" << std::endl;
  for (auto& shader : shaders) {
    std::cout << "Name: " << shader.first << std::endl;
    std::cout << " vertex: " << shader.second.vertexShaderPath << std::endl;
    std::cout << " fragment: " << shader.second.fragmentShaderPath << std::endl;
    if (!shader.second.geometricShaderPath.empty()) {
      std::cout << " geometric: " << shader.second.geometricShaderPath
                << std::endl;
    }
  }
}
}  // namespace al