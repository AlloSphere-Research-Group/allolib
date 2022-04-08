

#These should be provided by the CMakeFile for the library
# include_directories(${CMAKE_SOURCE_DIR}/external/Gamma)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/rtmidi)

# Google test
# Get and build googletest
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


set (gtest_src
    main.cpp
    src/test_presets.cpp
    src/test_file.cpp
    src/test_audio.cpp
    src/test_midi.cpp
    src/test_math.cpp
    src/test_mathSpherical.cpp
    src/test_mathSpherical.cpp
    src/test_osc.cpp
    src/test_lbap.cpp
    src/test_vbap.cpp

)

add_executable(al_gtests ${gtest_src})
set_target_properties(al_gtests PROPERTIES DEBUG_POSTFIX _debug)
set_target_properties(al_gtests PROPERTIES CXX_STANDARD 14)
set_target_properties(al_gtests PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(al_gtests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set_target_properties(al_gtests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/bin)
set_target_properties(al_gtests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/bin)

target_link_libraries(al_gtests PRIVATE gtest al)
add_test(NAME allolib_gtest COMMAND al_gtests)

if (ALLOLIB_RUN_TESTS)
add_custom_command( TARGET al_tests POST_BUILD
    COMMAND $<TARGET_FILE:al_tests>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin )
add_custom_command( TARGET al_gtests POST_BUILD
    COMMAND $<TARGET_FILE:al_gtests>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin )

endif(ALLOLIB_RUN_TESTS)
