
#include <cmath>

#include "al/io/al_File.hpp"
#include "catch.hpp"

using namespace al;

TEST_CASE("Conform directory") {
  std::string path = "c:/test/path/../otherpath/to/";
  std::string conformed = File::conformDirectory(path);

  REQUIRE(File::isSamePath(conformed, "c:/test/otherpath/to/"));
  //
  //
}
