set (test_src
    src/main.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/catch)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include ${dirs_to_include})

#These should be provided by the CMakeFile for the library
# include_directories(${CMAKE_SOURCE_DIR}/external/Gamma)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/rtmidi)

if (AL_WINDOWS)
  # CMAKE_BINARY_DIR: contains the driectory where cmake is run
  set(TEST_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${TEST_OUTPUT_PATH})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${TEST_OUTPUT_PATH})
endif (AL_WINDOWS)

add_executable(al_tests ${test_src})
set_target_properties(al_tests PROPERTIES DEBUG_POSTFIX _debug)
set_target_properties(al_tests PROPERTIES CXX_STANDARD 14)
set_target_properties(al_tests PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(al_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
set_target_properties(al_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
set_target_properties(al_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../bin)

target_link_libraries(al_tests al)
# if (AL_WINDOWS)
# target_link_libraries(
#   al_tests
#   debug ${al_path}/external/Gamma/lib/Gamma_debug.lib
#   optimized ${al_path}/external/Gamma/lib/Gamma.lib
# )
# else ()
# target_link_libraries(
#   al_tests
#   debug ${al_path}/external/Gamma/lib/libGamma_debug.a
#   optimized ${al_path}/external/Gamma/lib/libGamma.a
# )   
# endif ()
target_compile_definitions(al_tests PRIVATE ${definitions})

# if (WINDOWS)
# 
#   set(post_build_command
#     robocopy ${al_path}/dependencies/glew/bin/Release/x64 ${TEST_OUTPUT_PATH} glew32.dll &
#     robocopy ${al_path}/dependencies/glfw/lib-vc2015 ${TEST_OUTPUT_PATH} glfw3.dll &
#   )
# 
#   if (USE_APR)
#   	list(APPEND post_build_command
#   	  robocopy ${al_path}/dependencies/apr/ ${TEST_OUTPUT_PATH} libapr-1.dll &
#   	)
#   endif (USE_APR)
# 
#   if (USE_PORTAUDIO)
#   	list(APPEND post_build_command
#   	  robocopy ${al_path}/dependencies/portaudio/ ${TEST_OUTPUT_PATH} portaudio_x64.dll &
#   	)
#   endif (USE_PORTAUDIO)
# 
#   list(APPEND post_build_command
#   	IF %ERRORLEVEL% LEQ 1 exit 0
#   )
# 
#   add_custom_command(
#     TARGET al_tests
#     POST_BUILD
#     COMMAND ${post_build_command}
#   )
# endif (WINDOWS)
