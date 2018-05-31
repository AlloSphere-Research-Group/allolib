#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <cmath>
#include <iostream>

#include "al/core/math/al_Constants.hpp"
#include "al/core/system/al_Time.hpp"

//#include "test_audio.cpp"
//#include "test_midi.cpp"
//#include "test_math.cpp"
//#include "test_mathSpherical.cpp"
//#include "test_dynamicScene.cpp"
//#include "test_vbap.cpp"

int main( int argc, char* argv[] )
{
//    cleanDir("c:\\temp");
//    init (argv);
    int result = Catch::Session().run( argc, argv );
    return ( result < 0xff ? result : 0xff );
}
