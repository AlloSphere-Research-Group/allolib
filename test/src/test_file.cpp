
#include <cmath>

#include "al/io/al_File.hpp"
#include "catch.hpp"

using namespace al;

TEST_CASE("Conform directory") {
  std::string path = "c:/test/path/../otherpath/to/";
  std::string conformed = File::conformDirectory(path);

  REQUIRE(File::isSamePath(conformed, "c:/test/otherpath/to/"));

  path = "c:\\test\\path\\..\\otherpath\\to\\";
  conformed = File::conformDirectory(path);

  REQUIRE(File::isSamePath(conformed, "c:/test/otherpath/to/"));

  path = "c:\\test\\path/..\\otherpath\\to\\";
  conformed = File::conformDirectory(path);

  REQUIRE(File::isSamePath(conformed, "c:/test/otherpath/to/"));

  // spaces
  path = "c:\\test\\pa th/..\\other path\\to\\";
  conformed = File::conformDirectory(path);

  REQUIRE(File::isSamePath(conformed, "c:/test/other path/to/"));
}

TEST_CASE("Conform path to OS") {
  std::string path = "c:/test/path/../otherpath/to/";
  std::string conformed = File::conformPathToOS(path);

  REQUIRE(File::isSamePath(conformed, "c:/test/otherpath/to/"));

  path = "c:\\test\\path\\..\\otherpath\\to\\";
  conformed = File::conformPathToOS(path);

  REQUIRE(File::isSamePath(conformed, "c:/test/otherpath/to/"));

  path = "c:\\test\\path/..\\otherpath\\to\\";
  conformed = File::conformPathToOS(path);

  REQUIRE(File::isSamePath(conformed, "c:/test/otherpath/to/"));

  // spaces
  path = "c:\\test\\pa th/..\\other path\\to\\";
  conformed = File::conformPathToOS(path);

  REQUIRE(File::isSamePath(conformed, "c:/test/other path/to/"));

  // files
  path = "c:/test/path/../otherpath/to/file";
  conformed = File::conformPathToOS(path);

  REQUIRE(File::isSamePath(conformed, "c:/test/otherpath/to/file"));

  path = "c:\\test\\path\\..\\otherpath\\to\\file";
  conformed = File::conformPathToOS(path);

  REQUIRE(File::isSamePath(conformed, "c:/test/otherpath/to/file"));

  path = "c:\\test\\path/..\\otherpath\\to\\file";
  conformed = File::conformPathToOS(path);

  REQUIRE(File::isSamePath(conformed, "c:/test/otherpath/to/file"));

  // spaces
  path = "c:\\test\\pa th/..\\other path\\to\\file";
  conformed = File::conformPathToOS(path);

  REQUIRE(File::isSamePath(conformed, "c:/test/other path/to/file"));
}
