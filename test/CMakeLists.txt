set (test_src
    src/main.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/external/catch)
include_directories(${CMAKE_SOURCE_DIR}/include)

if (WINDOWS)
  # CMAKE_BINARY_DIR: contains the driectory where cmake is run
  set(TEST_OUTPUT_PATH ${CMAKE_BINARY_DIR}/test/)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${TEST_OUTPUT_PATH})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${TEST_OUTPUT_PATH})
endif (WINDOWS)

add_executable(al_tests ${test_src})
target_link_libraries(al_tests al)

if (WINDOWS)
	set(lib_path ${CMAKE_SOURCE_DIR}/dependencies)

  set(post_build_command
    robocopy ${lib_path}/apr/ ${TEST_OUTPUT_PATH} libapr-1.dll &
    robocopy ${lib_path}/glew/ ${TEST_OUTPUT_PATH} glew32.dll &
    robocopy ${lib_path}/glfw/ ${TEST_OUTPUT_PATH} glfw3.dll &
    robocopy ${lib_path}/portaudio/ ${TEST_OUTPUT_PATH} portaudio_x64.dll &
    IF %ERRORLEVEL% LEQ 1 exit 0
  )

  add_custom_command(
    TARGET al_tests
    POST_BUILD
    COMMAND ${post_build_command}
  )
endif (WINDOWS)