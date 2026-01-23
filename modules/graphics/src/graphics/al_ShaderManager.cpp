#include "al/graphics/al_ShaderManager.hpp"

#include <fstream>
#include <iostream>

namespace al {

void ShaderManager::setPollInterval(double interval)
{
  m_pollInterval = interval;

  for (auto& shader : m_shaders) {
    shader.second.fileWatcher->setPollInterval(interval);
  }
}

std::string ShaderManager::loadGLSL(const std::filesystem::path& filePath)
{
  if (filePath.empty()) {
    return "";
  }

  if (!std::filesystem::is_regular_file(filePath) &&
      !std::filesystem::is_symlink(filePath)) {
    std::cerr << "[ShaderManager] Incorrect filename: " << filePath
              << std::endl;
    return "";
  }

  std::ifstream in(filePath);
  if (!in) {
    std::cerr << "[ShaderManager] File not found: " << filePath << std::endl;
    return "";
  }

  std::string code;
  in.seekg(0, std::ios::end);
  code.resize(in.tellg());
  in.seekg(0, std::ios::beg);
  in.read(&code[0], code.size());
  in.close();

  std::string includeDirective{"#include \""};
  size_t includeStart = code.find(includeDirective);

  if (includeStart != std::string::npos) {
    size_t inclFileStart = includeStart + includeDirective.size();
    size_t includeEnd = code.find("\"", inclFileStart);

    std::string inclFileName =
        code.substr(inclFileStart, includeEnd - inclFileStart);
    std::filesystem::path inclFilePath = m_shaderDirectory / inclFileName;
    std::ifstream inclIn(inclFilePath);
    if (!inclIn) {
      std::cerr << "[ShaderManager] Include file not found: " << inclFilePath
                << std::endl;
      return "";
    }

    std::string replacement;
    inclIn.seekg(0, std::ios::end);
    replacement.resize(inclIn.tellg());
    inclIn.seekg(0, std::ios::beg);
    inclIn.read(&replacement[0], replacement.size());
    inclIn.close();

    code =
        code.replace(includeStart, includeEnd - includeStart + 2, replacement);
    std::cout << "[ShaderManager] Loading code from " << filePath << std::endl;
    std::cout << code << std::endl;
  }
  return code;
}

void ShaderManager::compile(ManagedShaderProgram& shaderProgram)
{
  std::string vertexShader = loadGLSL(shaderProgram.vertexPath);
  std::string fragmentShader = loadGLSL(shaderProgram.fragmentPath);
  std::string geometricShader = loadGLSL(shaderProgram.geometricPath);

  shaderProgram.program->compile(vertexShader, fragmentShader, geometricShader);
}

void ShaderManager::add(const std::string& programName,
                        const std::string& vertexShaderName,
                        const std::string& fragmentShaderName,
                        const std::string& geometricShaderName)
{
  auto search = m_shaders.find(programName);
  if (search != m_shaders.end()) {
    // m_shaders[programName].program->destroy();
    std::cerr << "[ShaderManager] Shader with same name already exists"
              << std::endl;
    return;
  }

  if (vertexShaderName.empty() || fragmentShaderName.empty()) {
    std::cerr << "[ShaderManager] Vertex / Fragment shader file required"
              << std::endl;
    return;
  }

  std::filesystem::path vPath = m_shaderDirectory / vertexShaderName;
  std::filesystem::path fPath = m_shaderDirectory / fragmentShaderName;

  if (!std::filesystem::exists(vPath) || !std::filesystem::exists(fPath)) {
    std::cerr << "[ShaderManager] Invalid vertex/fragment shader filename:"
              << std::endl
              << " vertex: " << vPath << std::endl
              << " fragment: " << fPath << std::endl;
    return;
  }

  for (const auto& entry :
       std::filesystem::recursive_directory_iterator(m_shaderDirectory)) {
    if (vertexShaderName.compare(entry.path().filename().string()) == 0) {
      vPath = entry;
      continue;
    }
    if (fragmentShaderName.compare(entry.path().filename().string()) == 0) {
      fPath = entry;
      continue;
    }
  }

  std::filesystem::path gPath = geometricShaderName.empty()
                                    ? std::filesystem::path()
                                    : m_shaderDirectory / geometricShaderName;

  m_shaders[programName] = {std::make_unique<ShaderProgram>(),
                            std::make_unique<FileWatcher>(), vPath, fPath,
                            gPath};

  auto& shaderProgram = m_shaders[programName];

  shaderProgram.fileWatcher->watch(shaderProgram.vertexPath);
  shaderProgram.fileWatcher->watch(shaderProgram.fragmentPath);
  if (!shaderProgram.geometricPath.empty()) {
    shaderProgram.fileWatcher->watch(shaderProgram.geometricPath);
  }

  shaderProgram.fileWatcher->setPollInterval(m_pollInterval);

  compile(shaderProgram);
}

bool ShaderManager::update()
{
  bool updated = false;
  for (auto& shader : m_shaders) {
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
  for (auto& shader : m_shaders) {
    std::cout << "Name: " << shader.first << std::endl;
    std::cout << " vertex: " << shader.second.vertexPath << std::endl;
    std::cout << " fragment: " << shader.second.fragmentPath << std::endl;
    if (!shader.second.geometricPath.empty()) {
      std::cout << " geometric: " << shader.second.geometricPath << std::endl;
    }
  }
}
}  // namespace al