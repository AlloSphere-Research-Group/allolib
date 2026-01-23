#include "al/io/al_FileWatcher.hpp"

#include <iostream>

namespace al {

bool FileWatcher::watch(const std::filesystem::path& file)
{
  auto search = watchedFiles.find(file);
  if (search != watchedFiles.end()) {
    std::cerr << "File already watched: " << file << std::endl;
    return false;
  }
  watchedFiles[file] = std::filesystem::last_write_time(file);
  return true;
}

bool FileWatcher::poll()
{
  bool changed = false;

  const auto& now = std::chrono::system_clock::now();

  if (now - lastPollTime > pollInterval) {
    lastPollTime = now;

    for (auto& [path, modified] : watchedFiles) {
      if (std::filesystem::last_write_time(path) > modified) {
        modified = std::filesystem::last_write_time(path);
        std::cout << "File modified: " << path << std::endl;
        changed = true;
      }
    }
  }

  return changed;
}

}  // namespace al