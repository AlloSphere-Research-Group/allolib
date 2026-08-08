cmake_minimum_required(VERSION 3.24...4.4)
project(allolib VERSION 3.0.0 LANGUAGES C CXX)

# Tells third-party submodules (Gamma, cpptoml, json) to
# override their cmake_minimum_required(VERSION < 3.5) policy.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(OpenGL_GL_PREFERENCE GLVND)

# Shared module helpers (al_configure_module, al_install_module).
include(${CMAKE_CURRENT_LIST_DIR}/cmake/AlloLibModule.cmake)

if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW) # allow linking a target defined in another directory
endif()

# When ON, vendored deps skip their own install(EXPORT) so AlloLib can bundle
# them into AlloLibBundledTargets (one export set per target). Requires the
# temporary patches in cmake/vendor-patches/ (./cmake/vendor-patches/apply.sh).
# A future vcpkg/Conan layout would leave this OFF and use find_dependency().
set(ALLOLIB_BUNDLE_DEPS ON)
# option(AL_VERBOSE_OUTPUT "" OFF)
option(ALLOLIB_ADD_EXAMPLES "" ON) # add example targets, but do not build by default
option(ALLOLIB_BUILD_EXAMPLES "" OFF) # if ON all example targets will be built by default
# option(ALLOLIB_ADD_EXTENSIONS "" OFF) # extension libraries will be added as targets
option(ALLOLIB_BUILD_TESTS "" OFF)

set(ALLOLIB_AUDIO_BACKEND "rtaudio" CACHE STRING
  "Audio backend: rtaudio, portaudio, or dummy")
set_property(CACHE ALLOLIB_AUDIO_BACKEND PROPERTY STRINGS rtaudio portaudio dummy)

# Deprecated booleans — mapped onto ALLOLIB_AUDIO_BACKEND if still set.
option(ALLOLIB_USE_PORTAUDIO "Use PortAudio instead of RtAudio (deprecated; use ALLOLIB_AUDIO_BACKEND)" OFF)
option(ALLOLIB_USE_DUMMY_AUDIO "Use Dummy Audio I/O (deprecated; use ALLOLIB_AUDIO_BACKEND)" OFF)
if(ALLOLIB_USE_DUMMY_AUDIO)
  set(ALLOLIB_AUDIO_BACKEND "dummy" CACHE STRING "Audio backend: rtaudio, portaudio, or dummy" FORCE)
elseif(ALLOLIB_USE_PORTAUDIO)
  set(ALLOLIB_AUDIO_BACKEND "portaudio" CACHE STRING "Audio backend: rtaudio, portaudio, or dummy" FORCE)
endif()

option(ALLOLIB_BUILD_SHARED "Build all libraries as shared libraries" OFF)
# option(TRAVIS_BUILD "" OFF)
# option(APPVEYOR_BUILD "" OFF)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(AL_MACOS 1 CACHE BOOL "Building on OS X")
  set(PLATFORM_DEFINITION -DAL_OSX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(AL_LINUX 1 CACHE BOOL "Building on Linux")
  set(PLATFORM_DEFINITION -DAL_LINUX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(AL_WINDOWS 1 CACHE BOOL "Building on Windows")
  set(PLATFORM_DEFINITION -DAL_WINDOWS)
  if (NOT ${CMAKE_GENERATOR_PLATFORM} STREQUAL "x64")
  message("Warning! Not building for Win64 : ${CMAKE_GENERATOR_PLATFORM}")
  endif(NOT ${CMAKE_GENERATOR_PLATFORM} STREQUAL "x64")
else ()
  message(FATAL_ERROR "system platform not defined")
endif ()

if(ALLOLIB_BUILD_SHARED)
    set(AL_LIBRARY_TYPE "SHARED")
else()
    set(AL_LIBRARY_TYPE "STATIC")
endif()


## Gamma ##
if(ALLOLIB_BUILD_SHARED)
    set(GAMMA_BUILD_SHARED ON CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
else(ALLOLIB_BUILD_SHARED)
    set(GAMMA_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif(ALLOLIB_BUILD_SHARED)

add_subdirectory(modules/external/Gamma)

## allolib core modules ##
add_subdirectory(modules/math)
add_subdirectory(modules/spatial)
add_subdirectory(modules/types)
add_subdirectory(modules/system)

add_subdirectory(modules/graphics)

add_subdirectory(modules/audio-io)
add_subdirectory(modules/audio)
if(ALLOLIB_AUDIO_BACKEND STREQUAL "dummy")
  add_subdirectory(modules/audio-backend-dummy)
elseif(ALLOLIB_AUDIO_BACKEND STREQUAL "portaudio")
  add_subdirectory(modules/audio-backend-portaudio)
elseif(ALLOLIB_AUDIO_BACKEND STREQUAL "rtaudio")
  add_subdirectory(modules/audio-backend-rtaudio)
else()
  message(FATAL_ERROR "Unknown ALLOLIB_AUDIO_BACKEND='${ALLOLIB_AUDIO_BACKEND}' (rtaudio, portaudio, dummy)")
endif()

# Convenience: AudioIO + the selected backend (no DSP). Typical apps also link al::audio.
add_library(al_audio_device INTERFACE)
add_library(al::audio_device ALIAS al_audio_device)
target_link_libraries(al_audio_device INTERFACE
  al::audio_io
  al::audio_backend
)
set_target_properties(al_audio_device PROPERTIES EXPORT_NAME audio_device)

add_subdirectory(modules/io)
add_subdirectory(modules/midi) # move to extension?
add_subdirectory(modules/serial) # move to extension?
add_subdirectory(modules/protocol)

add_subdirectory(modules/imgui)
add_subdirectory(modules/parameter) # composition: Parameter, presets, sequences
add_subdirectory(modules/ui) # graphics UI + scene (TODO split scene)
add_subdirectory(modules/config)
add_subdirectory(modules/app) # and sphere (TODO extract sphere to al_ext)

# Convenience umbrella targets. Prefer linking al::app (and explicit extras) in new code.
add_library(al_all INTERFACE)
add_library(al::all ALIAS al_all)
target_link_libraries(al_all INTERFACE
  al::app
  al::serial
)

# Legacy monolithic name used by older consumers.
add_library(al_legacy INTERFACE)
add_library(al ALIAS al_legacy)
target_link_libraries(al_legacy INTERFACE al::all)


## allolib extension modules ##
# if (ALLOLIB_ADD_EXTENSIONS)
  # message("including allolib extensions")
  # add_subdirectory(extensions)
  # add_subdirectory(../al_ext ../al_ext)
# endif()


# what is the purpose of these options in MSVC
# if (NOT ${CMAKE_BUILD_TYPE} STREQUAL Debug)
#     target_compile_options(al PUBLIC
#         $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
#         -Wall>
#         $<$<CXX_COMPILER_ID:MSVC>:
#         /wd4221 /wd4224 /wd4244  /wd4305  /wd4267 /wd4005 /wd4996>)
#     if (AL_WINDOWS)
#         set_target_properties(al PROPERTIES LINK_FLAGS "/ignore:4099")
#     endif(AL_WINDOWS)
# endif()



# who needs this currently? i see it used in examples?
# macro(Copy_dlls dest_path target DLLS_TO_COPY)
#   if (DLLS_TO_COPY)
#     foreach(LIBRARY ${DLLS_TO_COPY})
#       get_filename_component(LIBRARY_DIR "${LIBRARY}" DIRECTORY)
#       get_filename_component(LIBRARY_NAME "${LIBRARY}" NAME)
#       list(APPEND post_build_command robocopy "${LIBRARY_DIR}" "${app_path}" "${LIBRARY_NAME}" &)
#     endforeach(LIBRARY IN "${DLLS_TO_COPY}")

#     list(APPEND post_build_command IF %ERRORLEVEL% LEQ 1 exit 0)

#     add_custom_command(TARGET ${target} POST_BUILD COMMAND ${post_build_command})
#   endif ()
# endmacro(Copy_dlls)




if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # On macOS if jack is present, RtMidi will link to jack but not provide the
    # /usr/local/lib location as link directory. This statement only serves the
    # purpose of working around that bug. Perhaps can be removed on future
    # updates of RtMidi
    link_directories("/usr/local/lib")
    if(EXISTS "/opt/homebrew/lib")
      link_directories("/opt/homebrew/lib")
    endif()
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


# tests
if (ALLOLIB_BUILD_TESTS)
  message("including allolib tests")
  add_subdirectory(tests)
endif()

# include examples, so individual targets can be built
# use option ALLOLIB_BUILD_EXAMPLES to change whether examples are part of default build or must be specified by target
if (ALLOLIB_ADD_EXAMPLES)
  message("including allolib examples")
  add_subdirectory(examples)
endif()

# Installable CMake package: AlloLibConfig + exported targets.
include(${CMAKE_CURRENT_LIST_DIR}/cmake/AlloLibPackage.cmake)
