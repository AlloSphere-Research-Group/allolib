#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <cmath>
#include <iostream>

#include "al/math/al_Constants.hpp"
#include "al/system/al_Time.hpp"

int main(int argc, char *argv[]) {
  //    cleanDir("c:\\temp");
  //    init (argv);
  int result = Catch::Session().run(argc, argv);
  return (result < 0xff ? result : 0xff);
}
