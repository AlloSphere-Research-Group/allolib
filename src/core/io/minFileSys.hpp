#ifndef INLCUDE_MINIMAL_FILESYSTEM_HPP
#define INLCUDE_MINIMAL_FILESYSTEM_HPP

/*
    minimal and basic filesystem functions
    
    - for reading / writing file, use std::*stream
    - does not support unicode

        keehong youn, 2017, younkeehong@gmail.com
*/

#include <vector>
#include <string>

namespace minFileSys {
void readDir(const std::string& name, std::vector<std::string>& v);
bool pathExists(std::string const& path);
bool isPathDir(std::string const& path);
bool isDirEmpty(std::string const& dir_path);
void copyFile(std::string const& orig, std::string const& cpyd);
bool deleteFile(std::string const& f);
bool createDir(std::string const& dir_path);
bool deleteDir(std::string const& dir_path); // only for empty dir
bool deleteDirRecursively(std::string const& dir_path); // delete non-empty dir
std::string addPath(std::string parent, std::string const& child);
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
#include <locale>
#include <codecvt>
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
*/
#endif

#ifdef ITS_POSIX
#include <sys/types.h>
#include <dirent.h>
#include <cstdio> // std::remove
#include <sys/stat.h> // int stat(const char*, struct stat*), mkdir
#include <unistd.h> // rmdir
#endif

#include <fstream>
#include <iostream>
#include <cstring> // std::strcmp

namespace minFileSys {

std::string addPath(std::string parent, std::string const& child)
{
#ifdef ITS_WINDOWS
    if (parent[parent.size() - 1] != '\\') parent += "\\";
    parent += child;
    return parent;
#endif

#ifdef ITS_POSIX
    if (parent[parent.size() - 1] != '/') parent += "/";
    parent += child;
    return parent;
#endif
}

// http://www.martinbroadhurst.com/list-the-files-in-a-directory-in-c.html
void readDir(const std::string& name, std::vector<std::string>& v)
{
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

    if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
    {
        do {
            // unicode
            // v.push_back(converter.to_bytes(data.cFileName));

            v.push_back(data.cFileName);
        }
        while (FindNextFileA(hFind, &data) != 0);
        FindClose(hFind);
    }
#endif

#ifdef ITS_POSIX
    DIR* dirp = opendir(name.c_str());
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (std::strcmp(dp->d_name, ".") == 0) continue;
        if (std::strcmp(dp->d_name, "..") == 0) continue;
        v.push_back(dp->d_name);
    }
    closedir(dirp);
#endif
}

bool pathExists(std::string const& path)
{
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

bool isPathDir(std::string const& path)
{
#ifdef ITS_WINDOWS
    // unicode
    // auto attr = GetFileAttributes(path.c_str());

    auto attr = GetFileAttributesA(path.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) return false;
    return attr & FILE_ATTRIBUTE_DIRECTORY;
#endif

#ifdef ITS_POSIX
    struct stat result;
    if (stat(path.c_str(), &result) != 0) return false;
    return S_ISDIR(result.st_mode);
#endif
}

bool isDirEmpty(std::string const& dir_path)
{
#ifdef ITS_WINDOWS
    std::string pattern(dir_path);
    pattern.append("\\*");

    // unicode
    // std::wstring wide = converter.from_bytes(pattern);

    WIN32_FIND_DATAA data;
    HANDLE hFind;

    // unicode
    // if ((hFind = FindFirstFile(wide.c_str(), &data)) != INVALID_HANDLE_VALUE)

    if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
    {
        do {
            // does windows also have these aliases?
            if (std::strcmp(dp->d_name, ".") == 0) continue;
            if (std::strcmp(dp->d_name, "..") == 0) continue;
            FindClose(hFind);
            return false;
        }
        while (FindNextFileA(hFind, &data) != 0);
        FindClose(hFind);
        return true;
    }
#endif

#ifdef ITS_POSIX
    DIR* dirp = opendir(dir_path.c_str());
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (std::strcmp(dp->d_name, ".") == 0) continue;
        if (std::strcmp(dp->d_name, "..") == 0) continue;
        closedir(dirp);
        return false;
    }
    closedir(dirp);
    return true;
#endif
}

void copyFile(std::string const& orig, std::string const& cpyd)
{
    std::ifstream origStream{ orig, std::ios::binary };
    std::ofstream cpydStream{ cpyd, std::ios::binary };
    cpydStream << origStream.rdbuf();
    origStream.close();
    cpydStream.close();
}

bool deleteFile(std::string const& f)
{
#ifdef ITS_WINDOWS
    // unicode
    // DeleteFile(f.c_str());

    // If the function succeeds, the return value is nonzero.
    auto result = DeleteFileA(f.c_str());
    return !(result == 0);
#endif

#ifdef ITS_POSIX
    // 0​ upon success or non-zero value on error.
    auto result = std::remove(f.c_str());
    return (result == 0);
#endif
}

bool createDir(std::string const& dir_path)
{
    if (pathExists(dir_path)) {
        std::cout << "[!] [createDir] " << dir_path << " already exists" << std::endl;
        return true;
    }

#ifdef ITS_WINDOWS
    // TODO ...
    return false;
#endif

#ifdef ITS_POSIX
    // User: S_IRUSR (read), S_IWUSR (write), S_IXUSR (execute)
    // Group: S_IRGRP (read), S_IWGRP (write), S_IXGRP (execute)
    // Others: S_IROTH (read), S_IWOTH (write), S_IXOTH (execute)
    // Read + Write + Execute: S_IRWXU (User), S_IRWXG (Group), S_IRWXO (Others)
    const int dir_err = mkdir(dir_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); // 775
    if (-1 == dir_err) {
        std::cout << "[!] [createDir] Error creating directory " << dir_path << std::endl;
        return false;
    }
    return true;
#endif
}

bool deleteDir(std::string const& dir_path)
{
    if (!pathExists(dir_path)) {
        std::cout << "[!] [deleteDir] " << dir_path << " does not exist" << std::endl;
        return true;
    }

#ifdef ITS_WINDOWS
    // TODO ...
    return false;
#endif

#ifdef ITS_POSIX
    const int dir_err = rmdir(dir_path.c_str());
    if (-1 == dir_err) {
        std::cout << "[!] [deleteDir] Error deleting directory " << dir_path << std::endl;
        return false;
    }
    return true;
#endif
}

bool deleteDirRecursively(std::string const& dir_path)
{
    if (!pathExists(dir_path)) {
        std::cout << "[!] [deleteDirRecursively] " << dir_path << " does not exist" << std::endl;
        return true;
    }

    std::vector<std::string> files;
    readDir(dir_path, files);
    for (auto const& f : files) {
        auto added_path = addPath(dir_path, f);
        std::cout << added_path << std::endl;
        if (isPathDir(added_path)) {
            if (!deleteDirRecursively(added_path)) return false;
        }
        else {
            if (!deleteFile(added_path)) return false;
        }
    }
    
    return deleteDir(dir_path);
}


} // namespace minFileSys

#endif

// DOCUMENTATION ---------------------------------------------------------------

/*
    The mkdir() function shall fail if:

    [EACCES] Search permission is denied on a component of the path prefix,
            or write permission is denied on the parent directory of the directory to be created.
    [EEXIST] The named file exists.
    [ELOOP] A loop exists in symbolic links encountered during resolution of the path argument.
    [EMLINK] The link count of the parent directory would exceed {LINK_MAX}.
    [ENAMETOOLONG] The length of the path argument exceeds {PATH_MAX}
            or a pathname component is longer than {NAME_MAX}.
    [ENOENT] A component of the path prefix specified by path does not name
            an existing directory or path is an empty string.
    [ENOSPC] The file system does not contain enough space to hold
            the contents of the new directory or to extend the parent directory of the new directory.
    [ENOTDIR] A component of the path prefix is not a directory.
    [EROFS] The parent directory resides on a read-only file system.

    The mkdir() function may fail if:
    [ELOOP] More than {SYMLOOP_MAX} symbolic links were encountered during resolution of the path argument.
    [ENAMETOOLONG] As a result of encountering a symbolic link in resolution of
            the path argument, the length of the substituted pathname string exceeded {PATH_MAX}.
*/