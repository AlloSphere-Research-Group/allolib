# Get and build googletest
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Unit tests application
set (gtest_src
    main.cpp
    src/test_dynamic_scene.cpp
    src/test_parameter_server.cpp
    src/test_preset_sequencer.cpp
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
    src/test_speakers.cpp
)

add_executable(al_tests ${gtest_src})
set_target_properties(al_tests PROPERTIES DEBUG_POSTFIX _debug)
set_target_properties(al_tests PROPERTIES CXX_STANDARD 14)
set_target_properties(al_tests PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(al_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set_target_properties(al_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/bin)
set_target_properties(al_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/bin)

target_link_libraries(al_tests PRIVATE gtest al)

# Running the unit tests
add_test(NAME allolib_test COMMAND al_tests)

if (ALLOLIB_RUN_TESTS)
add_custom_command( TARGET al_tests POST_BUILD
    COMMAND $<TARGET_FILE:al_tests>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin )
endif(ALLOLIB_RUN_TESTS)
