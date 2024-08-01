#include "al/io/al_FileWatcher.hpp"

namespace al {

bool FileWatcher::watch(File &file) {
  if (auto search = watchedFiles.find(file.path());
      search != watchedFiles.end()) {
    std::cerr << "File is already being watched" << std::endl;
    return false;
  }
  watchedFiles[file.path()] = WatchedFile{file, file.modified()};
  return true;
}

bool FileWatcher::poll() {
  bool changed = false;

  // std::cout << "last: " << lastPollTime << std::endl;
  // std::cout << " system: " << al_system_time() << std::endl;
  if (al_system_time() > lastPollTime + pollInterval) {
    lastPollTime = al_system_time();

    for (auto &[path, watchedFile] : watchedFiles) {
      if (watchedFile.file.modified() > watchedFile.modified) {
        watchedFile.modified = watchedFile.file.modified();
        std::cout << "File modified: " << path << std::endl;
        changed = true;
      }
    }
  }

  return changed;
}

} // namespace al