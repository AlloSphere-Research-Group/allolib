# project's CMakeLists.txt file includes this script

# this script needs following to be predefined:
#         app_name
#         app_path
#         app_files_list
#         app_include_dirs (can be skipped)
#         app_link_libs (can be skipped)
#         al_path


include(${al_path}/cmake_modules/configure_platform.cmake)
# sets: AL_MACOS || AL_LINUX || AL_WINDOWS, and PLATFORM_DEFINITION
include(${al_path}/cmake_modules/find_core_dependencies.cmake)
# sets: CORE_INCLUDE_DIRS, CORE_LIBRARIES, CORE_LIBRARY_DIRS
include(${al_path}/cmake_modules/find_additional_dependencies.cmake)
# sets: ADDITIONAL_INCLUDE_DIRS, ADDITIONAL_LIBRARIES, ADDITIONAL_HEADERS,
#       ADDITIONAL_SOURCES, ADDITIONAL_DEFINITIONS
include(${al_path}/cmake_modules/external.cmake)
# sets: EXTERNAL_INCLUDE_DIRS, EXTERNAL_DEFINITIONS
#       EXTERNAL_LIBRARIES, EXTERNAL_DEBUG_LIBRARIES EXTERNAL_RELEASE_LIBRARIES
include(${al_path}/cmake_modules/basic_flags.cmake)
# sets: basic_flags

set(dirs_to_include
  ${al_path}/include
  ${app_include_dirs}
  ${CORE_INCLUDE_DIRS}
  ${ADDITIONAL_INCLUDE_DIRS}
  ${EXTERNAL_INCLUDE_DIRS}
)

set(libs_to_link
  ${app_link_libs}
  ${CORE_LIBRARIES}
  ${ADDITIONAL_LIBRARIES}
  ${EXTERNAL_LIBRARIES}
)

# set(debug_libs_to_link
#   ${EXTERNAL_DEBUG_LIBRARIES}
# )

# set(release_libs_to_link
#   ${EXTERNAL_RELEASE_LIBRARIES}
# )

set(definitions
  ${PLATFORM_DEFINITION}
  ${ADDITIONAL_DEFINITIONS}
  ${EXTERNAL_DEFINITIONS}
)

set(flags
  ${basic_flags}
)

link_directories(${CORE_LIBRARY_DIRS})

# --- setup app target ------------------------------------

# multi configuration generators
if (MSVC)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
endif ()

add_executable(${app_name} ${app_files_list})

set_target_properties(${app_name} PROPERTIES DEBUG_POSTFIX _debug)

#paths
set_target_properties(${app_name}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${app_path}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${app_path}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${app_path}/bin
)

# flags
target_compile_options(${app_name} PUBLIC ${flags}) # public because of headers

# c++14
set_target_properties(${app_name} PROPERTIES CXX_STANDARD 14)
set_target_properties(${app_name} PROPERTIES CXX_STANDARD_REQUIRED ON)

# definitions
target_compile_definitions(${app_name} PRIVATE ${definitions})

# include dirs
target_include_directories(${app_name} PRIVATE ${dirs_to_include})

# libs
if (AL_WINDOWS)
  target_link_libraries(${app_name} debug ${al_path}/lib/al_debug.lib optimized ${al_path}/lib/al.lib)
else()
  target_link_libraries(${app_name} debug ${al_path}/lib/libal_debug.a optimized ${al_path}/lib/libal.a)
endif (AL_WINDOWS)
target_link_libraries(${app_name} ${libs_to_link})
# target_link_libraries(
#   ${app_name}
#   debug ${debug_libs_to_link}
#   optimized ${release_libs_to_link}
# )

if (AL_WINDOWS)
  # when run from Visual Studio, working directory is where the solution is by default
  # set it to app output directory
  set_target_properties(${app_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${app_path}/bin)
  # startup project is `ALL_BUILD` by default so we change it to app project
  set_directory_properties(PROPERTIES VS_STARTUP_PROJECT ${app_name})

  # post build events for copying dlls
  set(post_build_command
    robocopy ${al_path}/dependencies/glew/bin/Release/x64 ${app_path}/bin glew32.dll &
    robocopy ${al_path}/dependencies/glfw/lib-vc2015 ${app_path}/bin glfw3.dll &
  )

  if (USE_PORTAUDIO)
    list(APPEND post_build_command
      robocopy ${al_path}/dependencies/portaudio/ ${app_path}/bin portaudio_x64.dll &
    )
  endif (USE_PORTAUDIO)

  if (USE_APR)
    list(APPEND post_build_command
      robocopy ${al_path}/dependencies/apr/ ${app_path}/bin libapr-1.dll &
    )
  endif (USE_APR)

  if (FREEIMAGE_FOUND)
    list(APPEND post_build_command
      robocopy ${al_path}/dependencies/FreeImage/Dist/x64 ${app_path}/bin FreeImage.dll &
    )
  endif (FREEIMAGE_FOUND)
  
  list(APPEND post_build_command
    IF %ERRORLEVEL% LEQ 1 exit 0
  )

  add_custom_command(
    TARGET ${app_name}
    POST_BUILD
    COMMAND ${post_build_command}
  )
endif (AL_WINDOWS)