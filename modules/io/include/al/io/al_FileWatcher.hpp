#ifndef AL_FILEWATCHER_HPP
#define AL_FILEWATCHER_HPP

#include <chrono>
#include <filesystem>
#include <map>
#include <string>

namespace al {

/// @brief Monitors files and notifies if the files have been modified
class FileWatcher {
 public:
  /// @brief lastPollTime is set as the current system clock
  FileWatcher() : lastPollTime(std::chrono::system_clock::now()) {}

  /// @brief add file to the watched file map
  /// @param file file object containing file information
  /// @return false if file is already being watched
  bool watch(const std::filesystem::path& file);

  /// @brief add file at the path to the watched file map
  /// @param path path to the file
  /// @return false if file is already being watched
  inline bool watch(const std::string& path)
  {
    std::filesystem::path filepath{path};
    return watch(filepath);
  }

  /// @brief remove file at the path from the watched file map
  /// @param path path to file
  inline void unwatch(const std::string& path)
  {
    std::filesystem::path filepath{path};
    watchedFiles.erase(filepath);
  }

  /// @brief remove file from the watched file map
  /// @param file file object containing file information
  inline void unwatch(const std::filesystem::path& file) { watchedFiles.erase(file); }

  /// @brief set the polling interval
  /// @param interval interval length
  inline void setPollInterval(const double interval)
  {
    std::chrono::duration<double> newInterval{interval};
    pollInterval = newInterval;
  }

  /// @brief poll the files to see if they've been modified
  /// @return true if any of the files have been modified
  bool poll();

 protected:
  /// @brief map of file path and file information
  std::map<std::filesystem::path, std::filesystem::file_time_type> watchedFiles;
  /// @brief last system time files have been polled
  std::chrono::system_clock::time_point lastPollTime;
  /// @brief interval of file polling
  std::chrono::duration<double> pollInterval{0};
};

}  // namespace al

#endif