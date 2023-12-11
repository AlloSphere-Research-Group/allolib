
#include <cmath>

#include "al/io/al_File.hpp"
#include "gtest/gtest.h"

using namespace al;

#ifdef AL_WINDOWS
TEST(File, ConformDirectory) {
  std::string path = "c:/test/path/../otherpath/to/";
  std::string conformed = File::conformDirectory(path);

  EXPECT_TRUE(File::isSamePath(conformed, "c:/test/otherpath/to/"));

  path = "c:\\test\\path\\..\\otherpath\\to\\";
  conformed = File::conformDirectory(path);

  EXPECT_TRUE(File::isSamePath(conformed, "c:/test/otherpath/to/"));

  path = "c:\\test\\path/..\\otherpath\\to\\";
  conformed = File::conformDirectory(path);

  EXPECT_TRUE(File::isSamePath(conformed, "c:/test/otherpath/to/"));

  // spaces
  path = "c:\\test\\pa th/..\\other path\\to\\";
  conformed = File::conformDirectory(path);

  EXPECT_TRUE(File::isSamePath(conformed, "c:/test/other path/to/"));
}

TEST(File, ConformPathToOS) {
  std::string path = "c:/test/path/../otherpath/to/";
  std::string conformed = File::conformPathToOS(path);

  EXPECT_TRUE(File::isSamePath(conformed, "c:/test/otherpath/to/"));

  path = "c:\\test\\path\\..\\otherpath\\to\\";
  conformed = File::conformPathToOS(path);

  EXPECT_TRUE(File::isSamePath(conformed, "c:/test/otherpath/to/"));

  path = "c:\\test\\path/..\\otherpath\\to\\";
  conformed = File::conformPathToOS(path);

  EXPECT_TRUE(File::isSamePath(conformed, "c:/test/otherpath/to/"));

  // spaces
  path = "c:\\test\\pa th/..\\other path\\to\\";
  conformed = File::conformPathToOS(path);

  EXPECT_TRUE(File::isSamePath(conformed, "c:/test/other path/to/"));

  // files
  path = "c:/test/path/../otherpath/to/file";
  conformed = File::conformPathToOS(path);

  EXPECT_TRUE(File::isSamePath(conformed, "c:/test/otherpath/to/file"));

  path = "c:\\test\\path\\..\\otherpath\\to\\file";
  conformed = File::conformPathToOS(path);

  EXPECT_TRUE(File::isSamePath(conformed, "c:/test/otherpath/to/file"));

  path = "c:\\test\\path/..\\otherpath\\to\\file";
  conformed = File::conformPathToOS(path);

  EXPECT_TRUE(File::isSamePath(conformed, "c:/test/otherpath/to/file"));

  // spaces
  path = "c:\\test\\pa th/..\\other path\\to\\file";
  conformed = File::conformPathToOS(path);

  EXPECT_TRUE(File::isSamePath(conformed, "c:/test/other path/to/file"));
}
#else
TEST(File, ConformDirectory) {
  std::string path = "/test/path/../otherpath/to/";
  std::string conformed = File::conformDirectory(path);

  EXPECT_TRUE(File::isSamePath(conformed, "/test/otherpath/to/"));

  // spaces
  path = "/test/pa th/../other path/to/";
  conformed = File::conformDirectory(path);

  EXPECT_TRUE(File::isSamePath(conformed, "/test/other path/to/"));
}

TEST(File, ConformPathToOS) {
  std::string path = "/test/path/../otherpath/to/";
  std::string conformed = File::conformPathToOS(path);

  EXPECT_TRUE(File::isSamePath(conformed, "/test/otherpath/to/"));

  // spaces
  path = "/test/pa th/../other path/to/";
  conformed = File::conformPathToOS(path);

  EXPECT_TRUE(File::isSamePath(conformed, "/test/other path/to/"));

  // files
  path = "/test/path/../otherpath/to/file";
  conformed = File::conformPathToOS(path);

  EXPECT_TRUE(File::isSamePath(conformed, "/test/otherpath/to/file"));

  // spaces
  path = "/test/pa th/../other path/to/file";
  conformed = File::conformPathToOS(path);

  EXPECT_TRUE(File::isSamePath(conformed, "/test/other path/to/file"));
}
#endif
