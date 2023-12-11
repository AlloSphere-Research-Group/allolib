#ifndef INLCUDE_MINIMAL_FILESYSTEM_HPP
#define INLCUDE_MINIMAL_FILESYSTEM_HPP

/*
    minimal and basic filesystem functions

    - for reading / writing file, use std::*stream
    - does not support unicode

        keehong youn, 2017, younkeehong@gmail.com
*/

#include <string>
#include <vector>

namespace minFileSys {
void readDir(const std::string &name, std::vector<std::string> &v);
bool pathExists(std::string const &path);
bool isPathDir(std::string const &path);
bool isDirEmpty(std::string const &dir_path);
void copyFile(std::string const &orig, std::string const &cpyd);
bool deleteFile(std::string const &f);
bool createDir(std::string const &dir_path);
bool deleteDir(std::string const &dir_path);            // only for empty dir
bool deleteDirRecursively(std::string const &dir_path); // delete non-empty dir
std::string addPath(std::string parent, std::string const &child);
std::string currentPath();
} // namespace minFileSys

// IMPLEMENTATION --------------------------------------------------------------

#if defined(_WIN32)
#define ITS_WINDOWS

#elif defined(__APPLE__)
#define ITS_MACOS
#define ITS_POSIX

#else
#define ITS_LINUX
#define ITS_POSIX
#endif

#ifdef ITS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
/*
// for unicode
#include <codecvt>
#include <locale>
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
*/
#endif

#ifdef ITS_POSIX
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h> // int stat(const char*, struct stat*), mkdir
#include <sys/types.h>
#include <unistd.h> // rmdir

#include <cstdio> // std::remove
#endif

#include <cstring> // std::strcmp
#include <fstream>
#include <iostream>

namespace minFileSys {

inline std::string addPath(std::string parent, std::string const &child) {
#ifdef ITS_WINDOWS
  if (parent[parent.size() - 1] != '\\' || parent[parent.size() - 1] != '\\')
    parent += "\\";
  parent += child;
  return parent;
#endif

#ifdef ITS_POSIX
  if (parent[parent.size() - 1] != '/')
    parent += "/";
  parent += child;
  return parent;
#endif
}

// http://www.martinbroadhurst.com/list-the-files-in-a-directory-in-c.html
inline void readDir(const std::string &name, std::vector<std::string> &v) {
  if (!pathExists(name)) {
    std::cout << "[!] [readDir] " << name << " does not exist" << std::endl;
    return;
  }
#ifdef ITS_WINDOWS
  std::string pattern(name);
  pattern.append("\\*");

  // unicode
  // std::wstring wide = converter.from_bytes(pattern);

  WIN32_FIND_DATAA data;
  HANDLE hFind;

  // unicode
  // if ((hFind = FindFirstFile(wide.c_str(), &data)) != INVALID_HANDLE_VALUE)

  if ((hFind = FindFirstFileA(pattern.c_str(), &data)) !=
      INVALID_HANDLE_VALUE) {
    do {
      if (std::strcmp(data.cFileName, ".") == 0)
        continue;
      if (std::strcmp(data.cFileName, "..") == 0)
        continue;
      // unicode
      // v.push_back(converter.to_bytes(data.cFileName));

      v.push_back(data.cFileName);
    } while (FindNextFileA(hFind, &data) != 0);
    FindClose(hFind);
  }
#endif

#ifdef ITS_POSIX
  DIR *dirp = opendir(name.c_str());
  struct dirent *dp;
  while ((dp = readdir(dirp)) != NULL) {
    if (std::strcmp(dp->d_name, ".") == 0)
      continue;
    if (std::strcmp(dp->d_name, "..") == 0)
      continue;
    v.push_back(dp->d_name);
  }
  closedir(dirp);
#endif
}

inline bool pathExists(std::string const &path) {
#ifdef ITS_WINDOWS
  // unicode
  // return !(INVALID_FILE_ATTRIBUTES == GetFileAttributes(path.c_str()));

  return !(INVALID_FILE_ATTRIBUTES == GetFileAttributesA(path.c_str()));
#endif

#ifdef ITS_POSIX
  struct stat result;
  return (stat(path.c_str(), &result) == 0);
#endif
}

inline bool isPathDir(std::string const &path) {
#ifdef ITS_WINDOWS
  // unicode
  // auto attr = GetFileAttributes(path.c_str());

  auto attr = GetFileAttributesA(path.c_str());
  if (attr == INVALID_FILE_ATTRIBUTES)
    return false;
  return attr & FILE_ATTRIBUTE_DIRECTORY;
#endif

#ifdef ITS_POSIX
  struct stat result;
  if (stat(path.c_str(), &result) != 0)
    return false;
  return S_ISDIR(result.st_mode);
#endif
}

inline bool isDirEmpty(std::string const &dir_path) {
#ifdef ITS_WINDOWS
  std::string pattern(dir_path);
  pattern.append("\\*");

  // unicode
  // std::wstring wide = converter.from_bytes(pattern);

  WIN32_FIND_DATAA data;
  HANDLE hFind;

  // unicode
  // if ((hFind = FindFirstFile(wide.c_str(), &data)) != INVALID_HANDLE_VALUE)

  if ((hFind = FindFirstFileA(pattern.c_str(), &data)) !=
      INVALID_HANDLE_VALUE) {
    do {
      if (std::strcmp(data.cFileName, ".") == 0)
        continue;
      if (std::strcmp(data.cFileName, "..") == 0)
        continue;
      FindClose(hFind);
      return false;
    } while (FindNextFileA(hFind, &data) != 0);
    FindClose(hFind);
    return true;
  }
#endif

#ifdef ITS_POSIX
  DIR *dirp = opendir(dir_path.c_str());
  struct dirent *dp;
  while ((dp = readdir(dirp)) != NULL) {
    if (std::strcmp(dp->d_name, ".") == 0)
      continue;
    if (std::strcmp(dp->d_name, "..") == 0)
      continue;
    closedir(dirp);
    return false;
  }
  closedir(dirp);
  return true;
#endif
}

inline void copyFile(std::string const &orig, std::string const &cpyd) {
  std::ifstream origStream{orig, std::ios::binary};
  std::ofstream cpydStream{cpyd, std::ios::binary};
  cpydStream << origStream.rdbuf();
  origStream.close();
  cpydStream.close();
}

inline bool deleteFile(std::string const &f) {
#ifdef ITS_WINDOWS
  // unicode
  // DeleteFile(f.c_str());

  // If the function succeeds, the return value is nonzero.
  auto result = DeleteFileA(f.c_str());
  return !(result == 0);
#endif

#ifdef ITS_POSIX
  // 0 upon success or non-zero value on error.
  auto result = std::remove(f.c_str());
  return (result == 0);
#endif
}

inline bool createDir(std::string const &path) {
  if (pathExists(path)) {
    std::cout << "[!] [createDir] " << path << " already exists" << std::endl;
    return true;
  }
  if (path.size() == 0) {
    return false;
  }

#ifdef ITS_WINDOWS
  // Create all intermediate dirs up to last one
  for (unsigned i = 0; i < path.size() - 1; ++i) {
    if (path[i] == '/' || path[i] == '\\') {
      if (CreateDirectory(path.substr(0, i + 1).c_str(), NULL) != 0) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
          std::cout << "[!] [createDir] Error creating directory " << path
                    << std::endl;
          return false;
        }
      }
    }
  }
  return CreateDirectory(path.c_str(), NULL) != 0;
#endif

#ifdef ITS_POSIX
  // User: S_IRUSR (read), S_IWUSR (write), S_IXUSR (execute)
  // Group: S_IRGRP (read), S_IWGRP (write), S_IXGRP (execute)
  // Others: S_IROTH (read), S_IWOTH (write), S_IXOTH (execute)
  // Read + Write + Execute: S_IRWXU (User), S_IRWXG (Group), S_IRWXO (Others)
  auto mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH; // 755

  // const int dir_err = mkdir(path.c_str(), mode);
  // if (-1 == dir_err) {
  //     std::cout << "[!] [createDir] Error creating directory " << path <<
  //     std::endl; return false;
  // }

  // ALLOSYSTEM CODE
  // recursively create directories
  for (unsigned i = 0; i < path.size() - 1; ++i) {
    if (path[i] == '/') {
      if (mkdir(path.substr(0, i + 1).c_str(), mode) != 0) {
        if (errno != EEXIST && errno != EISDIR) {
          std::cout << "[!] [createDir] Error creating directory " << errno
                    << " " << path << std::endl;
          return false;
        }
      }
    }
  }
  return mkdir(path.c_str(), mode) == 0;
#endif
}

inline bool deleteDir(std::string const &dir_path) {
  if (!pathExists(dir_path)) {
    std::cout << "[!] [deleteDir] " << dir_path << " does not exist"
              << std::endl;
    return true;
  }

#ifdef ITS_WINDOWS
  if (RemoveDirectory(dir_path.c_str()) != 0) {
    return true;
  }
  return false;
#endif

#ifdef ITS_POSIX
  const int dir_err = rmdir(dir_path.c_str());
  if (-1 == dir_err) {
    std::cout << "[!] [deleteDir] Error deleting directory " << dir_path
              << std::endl;
    return false;
  }
  return true;
#endif
}

#ifdef ITS_WINDOWS
inline bool move_files_to_temp_dir_recursively_and_delete_original_folder(
    std::string d, std::string tempd) {
  // std::cout << "move_files_to_temp_dir_recursively on " << d << std::endl;
  std::string pattern(d);
  pattern.append("\\*");
  WIN32_FIND_DATAA data;
  HANDLE hFind;
  if ((hFind = FindFirstFileA(pattern.c_str(), &data)) !=
      INVALID_HANDLE_VALUE) {
    do {
      if (std::strcmp(data.cFileName, ".") == 0)
        continue;
      if (std::strcmp(data.cFileName, "..") == 0)
        continue;

      std::string child = d;
      child.append("\\");
      child.append(data.cFileName);

      auto attr = GetFileAttributesA(child.c_str());
      bool is_dir = attr & FILE_ATTRIBUTE_DIRECTORY;
      // std::cout << data.cFileName << " is dir? " << (is_dir ? 1 : 0) <<
      // std::endl;
      if (is_dir) {
        bool result =
            move_files_to_temp_dir_recursively_and_delete_original_folder(
                child, tempd);
        if (!result)
          return false;
      } else {
        std::string new_file_path = tempd;
        new_file_path.append("\\");
        new_file_path.append("minfilesys_file_to_be_deleted_");
        new_file_path.append(data.cFileName);
        while (MoveFile(child.c_str(), new_file_path.c_str()) == 0) {
          auto err = GetLastError();
          if (err == ERROR_ALREADY_EXISTS) {
            new_file_path.append("1");
          } else {
            std::cout << "error moving file for deletion " << err << std::endl;
            return false;
          }
        }
        // std::cout << data.cFileName << std::endl;
      }
    } while (FindNextFileA(hFind, &data) != 0);

    FindClose(hFind);
  }
  return deleteDir(d);
}

inline bool delete_files_that_starts_with(std::string prefix) {
  std::string pattern(".\\");
  pattern.append(prefix);
  pattern.append("*");
  WIN32_FIND_DATAA data;
  HANDLE hFind;
  if ((hFind = FindFirstFileA(pattern.c_str(), &data)) !=
      INVALID_HANDLE_VALUE) {
    do {
      std::string child = ".\\";
      child.append(data.cFileName);
      // std::cout << child << std::endl;
      minFileSys::deleteFile(child);
    } while (FindNextFileA(hFind, &data) != 0);
    FindClose(hFind);
  }
  return true;
}
#endif // ITS_WINDOWS

inline bool deleteDirRecursively(std::string const &dir_path) {
  if (!pathExists(dir_path)) {
    std::cout << "[!] [deleteDirRecursively] " << dir_path << " does not exist"
              << std::endl;
    return true;
  }

#ifdef ITS_WINDOWS
  // https://stackoverflow.com/questions/734717/how-to-delete-a-folder-in-c
  // https://github.com/CppCon/CppCon2015/blob/master/Tutorials/Racing%20the%20Filesystem/Racing%20the%20Filesystem%20-%20Niall%20Douglas%20-%20CppCon%202015.pdf
  // It looks like this solution can fail due to filesystem races:
  // DeleteFile is not atomic, which means deleting the directory that
  // contained it can fail because the directory isn't (yet) empty. This talk
  // explains the problem in detail and gives a safer way to delete a
  // directory/tree on Windows: youtube.com/watch?v=uhRWMGBjlO8
  // - Adrian McCarthy

  // deleting a directory tree on Windows
  // 1. enumerate directory contents
  // 2. for every non-empty directory, recurse to step 1
  // 3. for every file, try to rename to random name in %TEMP and then delete
  //    mark for later deletion
  // 4. for every empty directory, rename to random name in %TEMP and then
  //    delete mark for later deletion
  // 5. loop the above until directory tree deleted
  //   (may take as long as any item opened without FILE_SHARE_DELETE is open)

  // move all the files to current dir and delete empty directory
  bool ret = move_files_to_temp_dir_recursively_and_delete_original_folder(
      dir_path, ".");
  // delete all files
  ret &= delete_files_that_starts_with("minfilesys_file_to_be_deleted_");
  return ret;
#endif

#ifdef ITS_POSIX
  std::vector<std::string> files;
  readDir(dir_path, files);
  for (auto const &f : files) {
    auto added_path = addPath(dir_path, f);
    std::cout << added_path << std::endl;
    if (isPathDir(added_path)) {
      if (!deleteDirRecursively(added_path))
        return false;
    } else {
      if (!deleteFile(added_path))
        return false;
    }
  }
  return deleteDir(dir_path);
#endif
}

} // namespace minFileSys

#if defined(ITS_WINDOWS)
#undef ITS_WINDOWS
#endif

#if defined(ITS_MACOS)
#undef ITS_MACOS
#endif

#if defined(ITS_POSIX)
#undef ITS_POSIX
#endif

#endif

// DOCUMENTATION ---------------------------------------------------------------

/*
    The mkdir() function shall fail if:

    [EACCES] Search permission is denied on a component of the path prefix,
            or write permission is denied on the parent directory of the
   directory to be created. [EEXIST] The named file exists. [ELOOP] A loop
   exists in symbolic links encountered during resolution of the path argument.
    [EMLINK] The link count of the parent directory would exceed {LINK_MAX}.
    [ENAMETOOLONG] The length of the path argument exceeds {PATH_MAX}
            or a pathname component is longer than {NAME_MAX}.
    [ENOENT] A component of the path prefix specified by path does not name
            an existing directory or path is an empty string.
    [ENOSPC] The file system does not contain enough space to hold
            the contents of the new directory or to extend the parent directory
   of the new directory. [ENOTDIR] A component of the path prefix is not a
   directory. [EROFS] The parent directory resides on a read-only file system.

    The mkdir() function may fail if:
    [ELOOP] More than {SYMLOOP_MAX} symbolic links were encountered during
   resolution of the path argument. [ENAMETOOLONG] As a result of encountering a
   symbolic link in resolution of the path argument, the length of the
   substituted pathname string exceeded {PATH_MAX}.
*/
