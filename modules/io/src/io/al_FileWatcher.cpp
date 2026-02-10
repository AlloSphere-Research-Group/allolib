#include "al/io/al_FileWatcher.hpp"

namespace al {

bool FileWatcher::watch(File& file)
{
  auto search = watchedFiles.find(file.path());
  if (search != watchedFiles.end()) {
    std::cerr << "File is already being watched" << std::endl;
    return false;
  }
  watchedFiles[file.path()] = WatchedFile{file, file.modified()};
  return true;
}

bool FileWatcher::poll()
{
  bool changed = false;

  // std::cout << "last: " << lastPollTime << std::endl;
  // std::cout << " system: " << al_system_time() << std::endl;
  if (al_system_time() > lastPollTime + pollInterval) {
    lastPollTime = al_system_time();

    for (auto& watchedFile : watchedFiles) {
      if (watchedFile.second.file.modified() > watchedFile.second.modified) {
        watchedFile.second.modified = watchedFile.second.file.modified();
        std::cout << "File modified: " << watchedFile.first << std::endl;
        changed = true;
      }
    }
  }

  return changed;
}

}  // namespace al