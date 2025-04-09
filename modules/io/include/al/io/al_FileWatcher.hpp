#ifndef AL_FILEWATCHER_HPP
#define AL_FILEWATCHER_HPP

#include <map>
#include <string>

#include "al/io/al_File.hpp"
#include "al/system/al_Time.hpp"

namespace al {

/// @brief Contains file information and modified time as a local variable
struct WatchedFile {
  File file;
  al_sec modified;
};

/// @brief Monitors files and notifies if the files have been modified
class FileWatcher {
public:
  /// @brief lastPollTime is set as the current timestamp
  FileWatcher() : lastPollTime(al_system_time()) {}

  /// @brief add file at the path to the watched file map
  /// @param path path to the file
  /// @return false if file is already being watched
  inline bool watch(const std::string &path) {
    File file(path);
    return watch(file);
  }

  /// @brief add file to the watched file map
  /// @param file file object containing file information
  /// @return false if file is already being watched
  bool watch(File &file);

  /// @brief remove file at the path from the watched file map
  /// @param path path to file
  inline void unwatch(const std::string &path) { watchedFiles.erase(path); }

  /// @brief remove file from the watched file map
  /// @param file file object containing file information
  inline void unwatch(File &file) { unwatch(file.path()); }

  /// @brief set the polling interval
  /// @param interval interval length
  inline void setPollInterval(const al_sec& interval) {
    pollInterval = interval;
  }

  /// @brief poll the files to see if they've been modified
  /// @return true if any of the files have been modified
  bool poll();

protected:
  /// @brief map of file path and file information
  std::map<std::string, WatchedFile> watchedFiles;
  /// @brief last system time files have been polled
  al_sec lastPollTime;
  /// @brief interval of file polling
  al_sec pollInterval{0};
};

} // namespace al

#endif