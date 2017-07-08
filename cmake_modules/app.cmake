include(${al_path}/cmake_modules/configure_platform.cmake)
# sets: MACOS || LINUX || WINDOWS_MINGW || WINDOWS, and PLATFORM_DEFINITION
include(${al_path}/cmake_modules/find_core_dependencies.cmake)
# sets: CORE_INCLUDE_DIRS, CORE_LIBRARIES
include(${al_path}/cmake_modules/find_additional_dependencies.cmake)
# sets: ADDITIONAL_INCLUDE_DIRS, ADDITIONAL_LIBRARIES, ADDITIONAL_HEADERS,
#       ADDITIONAL_SOURCES, ADDITIONAL_DEFINITIONS
include(${al_path}/cmake_modules/external.cmake)
# sets: EXTERNAL_LIBRARIES, EXTERNAL_DEFINITIONS

set(headers
  ${ADDITIONAL_HEADERS}
)

set(sources
  ${ADDITIONAL_SOURCES}
)

set(dirs_to_include
  ${al_path}/include
  ${CORE_INCLUDE_DIRS}
  ${ADDITIONAL_INCLUDE_DIRS}
)

set(libs_to_link
  ${CORE_LIBRARIES}
  ${ADDITIONAL_LIBRARIES}
  ${EXTERNAL_LIBRARIES}
)

set(definitions
  ${PLATFORM_DEFINITION}
  ${ADDITIONAL_DEFINITIONS}
  ${EXTERNAL_DEFINITIONS}
)

# for linking different libs for debug/release target
if (WINDOWS)
  set(al_lib_debug ${al_path}/al_debug.lib)
  set(al_lib_release ${al_path}/al.lib)
else()
  set(al_lib_debug ${al_path}/libal_debug.a)
  set(al_lib_release ${al_path}/libal.a)
endif (WINDOWS)

set(APP_OUTPUT_PATH ${app_path}/bin)
if (WINDOWS)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${APP_OUTPUT_PATH})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${APP_OUTPUT_PATH})
else ()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_OUTPUT_PATH})
endif (WINDOWS)

if (WINDOWS)
  # set(CMAKE_CXX_FLAGS "/W2")
else ()
  set(CMAKE_CXX_FLAGS "-std=c++14 -Wall ${CMAKE_CXX_FLAGS}")
endif (WINDOWS)

add_executable(${app_name} ${app_files_list})

set_target_properties(${app_name} PROPERTIES DEBUG_POSTFIX _debug)

if (WINDOWS)
  # when run from Visual Studio, working directory is where the solution is by default
  # set it to app output directory
  set_target_properties(${app_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${app_path}/bin)
endif (WINDOWS)

target_include_directories(${app_name} PRIVATE ${dirs_to_include})

target_link_libraries(${app_name} debug ${al_lib_debug} optimized ${al_lib_release})
target_link_libraries(${app_name} ${libs_to_link})

# post build events for windows
# copy dlls
if (WINDOWS)
  set(post_build_command
    robocopy ${al_path}/dependencies/glew/ ${APP_OUTPUT_PATH} glew32.dll &
    robocopy ${al_path}/dependencies/glfw/ ${APP_OUTPUT_PATH} glfw3.dll &
  )

  if (USE_PORTAUDIO)
  	list(APPEND post_build_command
  	  robocopy ${al_path}/dependencies/portaudio/ ${APP_OUTPUT_PATH} portaudio_x64.dll &
  	)
  endif (USE_PORTAUDIO)

  if (USE_APR)
  	list(APPEND post_build_command
	  robocopy ${al_path}/dependencies/apr/ ${APP_OUTPUT_PATH} libapr-1.dll &
  	)
  endif (USE_APR)

  list(APPEND post_build_command
  	IF %ERRORLEVEL% LEQ 1 exit 0
  )

  add_custom_command(
    TARGET ${app_name}
    POST_BUILD
    COMMAND ${post_build_command}
  )
endif (WINDOWS)