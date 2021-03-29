#include "al/io/al_File.hpp"

#include <stdlib.h> // realpath (POSIX), _fullpath (Windows)

#include "minFileSys.hpp"

#ifdef AL_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#include <direct.h>  // _getcwd
#include <windows.h> // TCHAR, LPCTSTR
#define platform_getcwd _getcwd
#ifndef PATH_MAX
#define PATH_MAX 260
#endif
#undef NOMINMAX
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> // getcwd (POSIX)
#define platform_getcwd getcwd
#endif

// For PushDirectory
#if defined(AL_WINDOWS)
#define chdir _chdir
#define getcwd _getcwd
#define popen _popen
#define pclose _pclose
#include <fileapi.h>
#endif

#include <cstring>
#include <iostream>
#include <memory>

#ifdef __GNUG__
#include <cxxabi.h>

#include <cstdlib>
#include <memory>

std::string al::demangle(const char *name) {
  int status = -4; // some arbitrary value to eliminate the compiler warning

  // enable c++11 by passing the flag -std=c++11 to g++
  std::unique_ptr<char, void (*)(void *)> res{
      abi::__cxa_demangle(name, NULL, NULL, &status), std::free};

  return (status == 0) ? res.get() : name;
}

#else

#if AL_WINDOWS

// does nothing if not g++
std::string al::demangle(const char *name) {
  // Windows prepends "struct " or "class " here, so remove it
  auto demangled = std::string(name);
  return demangled.substr(demangled.find(' ') + 1);
}

#else
// does nothing if not g++
std::string al::demangle(const char *name) { return name; }
#endif

#endif

namespace al {

// void path2dir(char* dst, const char* src) {
//    char* s;
//  snprintf(dst, AL_PATH_MAX, "%s", src);
//    s = strrchr(dst, '/');
//    if (s)
//        s[1] = '\0';
//    else
//        dst[0] = '\0';
//}

File::File(std::string path, std::string mode, bool open_)
    : mPath(path), mMode(mode), mContent(0), mSizeBytes(0), mFP(0) {
  if (open_)
    open();
}

File::File(const FilePath &path, std::string mode, bool open_)
    : mPath(path.filepath()), mMode(mode), mContent(0), mSizeBytes(0), mFP(0) {
  if (open_)
    open();
}

File::~File() {
  close();
  freeContent();
}

void File::allocContent(int n) {
  if (mContent)
    freeContent();
  mContent = new char[n + 1];
  mContent[n] = '\0';
}

void File::freeContent() { delete[] mContent; }

void File::getSize() {
  int r = 0;
  if (opened()) {
    fseek(mFP, 0, SEEK_END);
    r = ftell(mFP);
    rewind(mFP);
    //    printf("File::getSize %d (%s)\n", r, mode().c_str());
  }
  mSizeBytes = r;
}

bool File::open(const std::string &path_, const std::string &mode_) {
  path(path_);
  mode(mode_);
  return open();
}

bool File::open() {
  if (0 == mFP) {
    mFP = fopen(path().c_str(), mode().c_str());
    if (mFP) {
      getSize();
      return true;
    }
  }
  return false;
}

void File::close() {
  if (opened())
    fclose(mFP);
  mFP = 0;
  mSizeBytes = 0;
}

const char *File::readAll() {
  if (opened() && mMode[0] == 'r') {
    int n = size();
    // printf("reading %d bytes from %s\n", n, path().c_str());
    allocContent(n);
    int numRead = fread(mContent, sizeof(char), n, mFP);
    if (numRead < n) {
      // printf("warning: only read %d bytes\n", numRead);
    }
  }
  return mContent;
}

std::string File::read(const std::string &path) {
  File f(path, "rb");
  f.open();
  auto str = f.readAll();
  return str ? str : "";
}

int File::write(const std::string &path, const void *v, int size, int items) {
  File f(path, "w");
  int r = 0;
  if (f.open()) {
    r = f.write(v, size, items);
    f.close();
  }
  return r;
}

int File::write(const std::string &path, const std::string &data) {
  return File::write(path, &data[0], data.size());
}

bool File::remove(const std::string &path) {
  if (!File::isDirectory(path)) {
    // return ::remove(path.c_str()) == 0;
    return minFileSys::deleteFile(path.c_str());
  }
  return false;
}

bool File::copy(const std::string &srcPath, const std::string &dstPath,
                unsigned int bufferSize) {
  std::unique_ptr<char> buffer(new char[bufferSize]);
  if (!File::exists(srcPath) || File::isDirectory(srcPath)) {
    return false;
  }
  std::string outPath = dstPath;
  if (File::isDirectory(outPath)) {
    outPath += "/" + File::baseName(srcPath);
  }
  outPath = File::conformPathToOS(outPath);
  File srcFile(srcPath, "rb");
  if (!srcFile.open()) {
    return false;
  }
  File dstFile(outPath, "wb");
  if (!dstFile.open()) {
    return false;
  }
  int bytesRead = 0;
  unsigned int totalBytes = 0;
  while ((bytesRead = srcFile.read(buffer.get(), 1, bufferSize)) > 0) {
    dstFile.write(buffer.get(), bytesRead);
    totalBytes += bytesRead;
  }
  bool writeComplete = (totalBytes == srcFile.size());
  srcFile.close();
  dstFile.close();

  return writeComplete;
}

std::string File::conformDirectory(const std::string &path) {
  if (path[0]) {
    std::string newpath = path;
    if (!al::File::isRelativePath(path)) {
      newpath = absolutePath(path);
    }
    if (AL_FILE_DELIMITER != newpath[newpath.size() - 1]) {
      return newpath + AL_FILE_DELIMITER;
    }
    return newpath;
  }
  return "." AL_FILE_DELIMITER_STR;
}

std::string File::conformPathToOS(const std::string &path) {
  std::string res;

  // Ensure delimiters are correct
  bool previousIsDelimiter = false;
  bool previousIsDot = false;
  for (unsigned i = 0; i < path.size(); ++i) {
    char c = path[i];
    if ('\\' == c || '/' == c) {
      if (previousIsDot) {
        // ./ - remove dot and dom't append separator
        res.resize(res.size() - 1);
        previousIsDot = false;
        previousIsDelimiter = false;
      } else {
        if (!previousIsDelimiter) {
          res += AL_FILE_DELIMITER;
          previousIsDelimiter = true;
        } else {
          previousIsDelimiter = false;
        }
      }
    } else {
      if (c == '.' && i > 0) { // Don't remove first dot
        previousIsDot = true;
      } else {
        previousIsDot = false;
      }
      res += path[i];
      previousIsDelimiter = false;
    }
  }

  // Ensure that directories end with a delimiter
  if (isDirectory(res)) {
    return conformDirectory(res);
  }
  return res;
}

std::string File::absolutePath(const std::string &path) {
#ifdef AL_WINDOWS
  TCHAR dirPart[4096];
  TCHAR **filePart = {NULL};
  GetFullPathName((LPCTSTR)path.c_str(), sizeof(dirPart), dirPart, filePart);
  std::string result = (char *)dirPart;
  if (filePart != NULL && *filePart != 0) {
    result += (char *)*filePart;
  }
  return result;
#else
  char temp[PATH_MAX];
  char *result = realpath(path.c_str(), temp);
  return result ? result : "";
#endif
}

bool File::isRelativePath(const std::string &path) {
  if (path.size() > 0) {
    if (path[0] == '/') {
      return false;
    }
    if (path.size() > 3 && ((path[0] >= 'a' && path[0] <= 'z') ||
                            (path[0] >= 'A' && path[0] <= 'Z'))) {
      if (path[1] == ':' && ((path[2] == '/' || path[2] == '\\'))) {
        return false;
      }
    }
  }
  return true;
}

std::string File::currentPath() {
  // Can be removed once moved to C++17 std::file_system::current_path()
  char buf[FILENAME_MAX];
  platform_getcwd(buf, FILENAME_MAX);
  std::string currentDir(buf);
  return File::conformPathToOS(currentDir);
}

bool File::isSamePath(const std::string &path1, const std::string &path2) {
  std::string newpath1 = conformPathToOS(path1);
  std::string newpath2 = conformPathToOS(path1);
  return newpath1 == newpath2;
}

std::string File::baseName(const std::string &path, const std::string &suffix) {
  auto posSlash = path.find_last_of("/\\"); // handle '/' or '\' path delimiters
  if (path.npos == posSlash)
    posSlash = 0; // no slash
  else
    ++posSlash;
  auto posSuffix = suffix.empty() ? path.npos : path.find(suffix, posSlash);
  auto len = path.npos;
  if (path.npos != posSuffix)
    len = posSuffix - posSlash;
  return path.substr(posSlash, len);
}

std::string File::directory(const std::string &path) {
  size_t pos = path.find_last_of(AL_FILE_DELIMITER);
  if (std::string::npos != pos) {
    return path.substr(0, pos + 1);
  }
  return "." AL_FILE_DELIMITER_STR;
}

std::string File::extension(const std::string &path) {
  size_t pos = path.find_last_of('.');
  if (path.npos != pos) {
    return path.substr(pos);
  }
  return "";
}

static std::string stripEndSlash(const std::string &path) {
  if (path.size() < 2) {
    return path;
  }
  if (path.back() == '\\' || path.back() == '/') {
    return path.substr(0, path.size() - 1);
  }
  return path;
}

bool File::exists(const std::string &path) {
  // struct stat s;
  // return ::stat(stripEndSlash(path).c_str(), &s) == 0;
  return minFileSys::pathExists(path);
}

bool File::isDirectory(const std::string &path) {
  struct stat s;
  if (0 == ::stat(stripEndSlash(path).c_str(), &s)) { // exists?
    if (s.st_mode & S_IFDIR) {                        // is dir?
      return true;
    }
  }
  // if(s.st_mode & S_IFREG) // is file?
  return false;
}

bool File::searchBack(std::string &prefixPath, const std::string &matchPath,
                      int maxDepth) {
  if (prefixPath[0]) {
    prefixPath = conformDirectory(prefixPath);
  }
  int i = 0;
  for (; i < maxDepth; ++i) {
    if (File::exists(prefixPath + matchPath))
      break;
    prefixPath += ".." AL_FILE_DELIMITER_STR;
  }
  return i < maxDepth;
}

bool File::searchBack(std::string &path, int maxDepth) {
  std::string prefix = "";
  bool r = searchBack(prefix, path);
  if (r)
    path = prefix + path;
  return r;
}

al_sec al::File::modificationTime(const char *path) {
  struct stat s;
  if (::stat(path, &s) == 0) {
    // const auto& t = s.st_mtim;
    // return t.tv_sec + t.tv_usec/1e9;
    return s.st_mtime;
  }
  return 0.;
}

FilePath::FilePath(const std::string &file, const std::string &path)
    : mPath(path), mFile(file) {
  mPath = File::conformPathToOS(mPath);
}

FilePath::FilePath(const std::string &fullpath) {
  size_t found = fullpath.rfind(AL_FILE_DELIMITER);
  if (found != std::string::npos) {
    mPath = fullpath.substr(0, found + 1);
    mFile = fullpath.substr(found + 1);
  } else {
    mPath = AL_FILE_DELIMITER_STR;
    mFile = fullpath;
  }
}

std::mutex PushDirectory::mDirectoryLock;

PushDirectory::PushDirectory(std::string directory, bool verbose)
    : mVerbose(verbose) {
  mDirectoryLock.lock();
  getcwd(previousDirectory, sizeof(previousDirectory));
  if (directory.size() == 0) {
    return;
  }
  chdir(directory.c_str());
  if (mVerbose) {
    std::cout << "Pushing directory: " << directory << std::endl;
    char curDir[4096];
    getcwd(curDir, sizeof(curDir));
    std::cout << "now at: " << curDir << std::endl;
  }
}

PushDirectory::~PushDirectory() {
  if (strlen(previousDirectory) > 0) {
    chdir(previousDirectory);
    if (mVerbose) {
      std::cout << "Setting directory back to: " << previousDirectory
                << std::endl;
    }
  }
  mDirectoryLock.unlock();
}

// bool Dir::make(const std::string& path, bool recursive)
bool Dir::make(const std::string &path) {
  // return Impl().make(path, -1, recursive);
  return minFileSys::createDir(path);
}

bool Dir::remove(const std::string &path) {
  // return Impl().remove(path);
  return minFileSys::deleteDir(path);
}

bool Dir::removeRecursively(const std::string &path) {
  // return Impl().removeRecursively(path);
  return minFileSys::deleteDirRecursively(path);
}

void FileList::sort() {
  std::sort(mFiles.begin(), mFiles.end(),
            [](const FilePath &a, const FilePath &b) -> bool {
              return a.filepath() > b.filepath();
            });
}

void FileList::print(std::ostream &stream) const {
  stream << "FileList:" << std::endl;
  std::vector<FilePath>::const_iterator it = mFiles.begin();
  while (it != mFiles.end()) {
    const FilePath &f = (*it++);
    stream << f.filepath() << std::endl;
  }
}

SearchPaths::SearchPaths(const std::string &file) {
  FilePath fp(file);
  addAppPaths(fp.path());
}

SearchPaths::SearchPaths(int argc, char *const argv[], bool recursive) {
  addAppPaths(argc, argv, recursive);
}

SearchPaths::SearchPaths(const SearchPaths &cpy)
    : mSearchPaths(cpy.mSearchPaths), mAppPath(cpy.mAppPath) {}

void SearchPaths::addAppPaths(std::string path, bool recursive) {
  std::string filepath = File::directory(path);
  mAppPath = filepath;
  addSearchPath(filepath, recursive);
}

void SearchPaths::addAppPaths(int argc, const char **argv, bool recursive) {
  addAppPaths(recursive);
  if (argc > 0) {
    addAppPaths(File::directory(argv[0]), recursive);
  }
}

void SearchPaths::addAppPaths(int argc, char *const argv[], bool recursive) {
  addAppPaths(recursive);
  if (argc > 0) {
    addAppPaths(File::directory(argv[0]), recursive);
  }
}

void SearchPaths::addAppPaths(bool recursive) {
  char cwd[4096];
  if (platform_getcwd(cwd, sizeof(cwd))) {
    mAppPath = std::string(cwd) + "/";
    addSearchPath(mAppPath, recursive);
  }
}

void SearchPaths::addSearchPath(const std::string &src, bool recursive) {
  std::string path = File::conformDirectory(src);

  // check for duplicates
  std::list<searchpath>::iterator iter = mSearchPaths.begin();
  while (iter != mSearchPaths.end()) {
    // printf("path %s\n", iter->first.c_str());
    if (path == iter->first) {
      return;
    }
    iter++;
  }
  //  printf("adding path %s\n", path.data());
  mSearchPaths.push_front(searchpath(path, recursive));
}

void SearchPaths::print(std::ostream &stream) const {
  stream << "SearchPath " << this << " appPath: " << appPath() << std::endl;
  std::list<searchpath>::const_iterator it = mSearchPaths.begin();
  while (it != mSearchPaths.end()) {
    const SearchPaths::searchpath &sp = (*it++);
    stream << "SearchPath" << this << " appPath: " << sp.first
           << "(recursive: " << sp.second << ")" << std::endl;
  }
}

FilePath SearchPaths::find(const std::string &filename) {
  for (auto const &s : mSearchPaths) {
    auto const path = s.first;
    if (!minFileSys::pathExists(path)) {
      continue;
    }
    if (!minFileSys::isPathDir(path)) {
      continue;
    }
    auto result = searchFileFromDir(filename, path);
    if (result.valid()) {
      return result;
    }
  }
  return FilePath();
}

FileList SearchPaths::filter(bool (*f)(FilePath const &)) {
  FileList filtered;
  for (auto const &s : mSearchPaths) {
    auto const path = s.first;
    if (!minFileSys::pathExists(path)) {
      continue;
    }
    if (!minFileSys::isPathDir(path)) {
      continue;
    }
    filtered.add(filterInDir(path, f));
  }
  return filtered;
}

FileList SearchPaths::listAll() {
  FileList fileList;
  for (auto const &s : mSearchPaths) {
    auto const path = s.first;
    if (!minFileSys::pathExists(path)) {
      continue;
    }
    if (!minFileSys::isPathDir(path)) {
      continue;
    }
    fileList.add(fileListFromDir(path));
  }
  return fileList;
}

FileList itemListInDir(std::string const &dir) {
  auto dir_ = File::conformDirectory(dir);
  FileList fileList;
  std::vector<std::string> children;
  minFileSys::readDir(dir, children);
  for (auto const &c : children) {
    if (c == "." || c == "..") {
      continue;
    }
    fileList.add(FilePath{c, dir_});
  }
  return fileList;
}

FileList fileListFromDir(std::string const &dir) {
  auto dir_ = File::conformDirectory(dir);
  FileList fileList;
  std::vector<std::string> children;
  minFileSys::readDir(dir, children);
  for (auto const &c : children) {
    if (c == "." || c == "..") {
      continue;
    }
    if (minFileSys::isPathDir(dir_ + c)) {
      // std::cout << (dir + AL_FILE_DELIMITER_STR + c)
      //           << " is path, recursing." << std::endl;
      fileList.add(fileListFromDir((dir_ + c)));
    } else {
      fileList.add(FilePath{c, dir_});
    }
  }
  return fileList;
}

FilePath searchFileFromDir(std::string const &filename,
                           std::string const &dir) {
  auto dir_ = File::conformDirectory(dir);
  std::vector<std::string> children;
  minFileSys::readDir(dir_, children);
  for (auto const &c : children) {
    if (c == "." || c == "..") {
      continue;
    }
    if (minFileSys::isPathDir(dir_ + c)) {
      auto result = searchFileFromDir(filename, dir_ + c);
      if (result.valid()) {
        return result;
      }
      continue;
    }
    if (c == filename) {
      return FilePath(filename, dir_);
    }
  }
  return FilePath();
}

FileList filterInDir(std::string const &dir,
                     std::function<bool(FilePath const &)> f, bool recursive) {
  FileList filtered;
  auto dir_ = File::conformDirectory(dir);
  std::vector<std::string> children;
  minFileSys::readDir(dir_, children);
  for (auto const &c : children) {
    if (c == "." || c == "..") {
      continue;
    }
    if (recursive) {
      if (minFileSys::isPathDir(dir_ + c)) {
        filtered.add(filterInDir(dir_ + c, f));
        continue;
      }
    }
    auto fp = FilePath{c, dir_};
    if (f(fp)) {
      filtered.add(fp);
    }
  }
  return filtered;
}

bool checkExtension(std::string const &filename, std::string const &extension) {
  int filelen = filename.size();
  int extlen = extension.size();
  if (filelen <= extlen) {
    return false;
  }
  if (filename.substr(filelen - extlen) == extension) {
    return true;
  }
  return false;
}

bool checkExtension(FilePath const &filepath, std::string const &extension) {
  return checkExtension(filepath.file(), extension);
}

} // namespace al
