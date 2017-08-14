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
void copyFile(std::string const& orig, std::string const& cpyd);
void deleteFile(std::string const& f);
} // namespace minFileSys

// IMPLEMENTATION ------------------------------------------

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
#include <sys/stat.h> // int stat(const char*, struct stat*);
#endif

#include <fstream>

namespace minFileSys {

// http://www.martinbroadhurst.com/list-the-files-in-a-directory-in-c.html
void readDir(const std::string& name, std::vector<std::string>& v)
{
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

void copyFile(std::string const& orig, std::string const& cpyd)
{
	std::ifstream origStream{ orig, std::ios::binary };
	std::ofstream cpydStream{ cpyd, std::ios::binary };
	cpydStream << origStream.rdbuf();
	origStream.close();
	cpydStream.close();
}

void deleteFile(std::string const& f)
{
#ifdef ITS_WINDOWS
	// unicode
	// DeleteFile(f.c_str());

	DeleteFileA(f.c_str());
#endif

#ifdef ITS_POSIX
	std::remove(f.c_str());
#endif
}

} // namespace minFileSys

#endif