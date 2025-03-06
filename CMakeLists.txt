# allolib/CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(allolib VERSION 0.1.0)

# option(AL_VERBOSE_OUTPUT "" OFF)
option(ALLOLIB_ADD_EXAMPLES "" ON) # add example targets, but do not build by default
option(ALLOLIB_BUILD_EXAMPLES "" OFF) # if ON all example targets will be built by default
# option(ALLOLIB_ADD_EXTENSIONS "" OFF) # extension libraries will be added as targets
option(ALLOLIB_BUILD_TESTS "" OFF)
option(ALLOLIB_USE_PORTAUDIO "Use PortAudio instead of RtAudio" OFF) # TODO remove see below
option(ALLOLIB_USE_DUMMY_AUDIO "Use Dummy Audio I/O" OFF) # TODO remove see below

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

add_subdirectory(modules/audio/gamma/external/Gamma)

## allolib core modules ##
add_subdirectory(modules/core/math)
add_subdirectory(modules/core/spatial)
add_subdirectory(modules/core/types)
add_subdirectory(modules/core/system)

add_subdirectory(modules/graphics)

add_subdirectory(modules/audio/core)
if(ALLOLIB_USE_DUMMY_AUDIO) # TODO better way to select audio backend rtaudio/portaudio/dummy
  add_subdirectory(modules/audio/backend/dummy) 
elseif(ALLOLIB_USE_PORTAUDIO)
  add_subdirectory(modules/audio/backend/portaudio) 
else()  
  add_subdirectory(modules/audio/backend/rtaudio) 
endif()

add_subdirectory(modules/io)
add_subdirectory(modules/protocol/midi)
add_subdirectory(modules/protocol/osc)

add_subdirectory(modules/ui/imgui)
add_subdirectory(modules/ui/ui) # and scene (interdependent, TODO split up)

add_subdirectory(modules/util/config)
add_subdirectory(modules/util/sphere)

add_subdirectory(modules/domain)
add_subdirectory(modules/app)

add_subdirectory(modules/extensions/serial)


## allolib extension modules ##
# if (ALLOLIB_ADD_EXTENSIONS)
  # message("including allolib extensions")
  # add_subdirectory(extensions)
  # add_subdirectory(../al_ext ../al_ext)
# endif()


# no longer have single library for allolib, but we could use something like libtool to pack them all together if needed
# add_library(al ${AL_LIBRARY_TYPE} ${headers} ${sources})
# add_library(al STATIC dummy.cpp)
# target_link_libraries(al PUBLIC almath alspatial alsystem altypes algraphics alaudio alsound alio almidi alserial alprotocol alimgui alui alconfig alapp)

# set_target_properties(al PROPERTIES
#   DEBUG_POSTFIX d
#   CXX_STANDARD 14
#   CXX_STANDARD_REQUIRED ON
# )

# if (AL_WINDOWS)
#   target_compile_definitions(al PUBLIC NOMINMAX)
# else()
#     target_compile_options(al PRIVATE "-Wall")
# endif (AL_WINDOWS)

# TODO remove this by making separate modules for rtaudio / portaudio that supply audio domains
# if (ALLOLIB_USE_PORTAUDIO)
#     # TODO needs more work on Windows
#     if(ALLOLIB_USE_DUMMY_AUDIO)
#         message("ALLOLIB_USE_DUMMY_AUDIO ignored. Using PortAudio")
#     endif(ALLOLIB_USE_DUMMY_AUDIO)
#     target_compile_definitions(al PUBLIC ${PLATFORM_DEFINITION} AL_AUDIO_PORTAUDIO)
#     target_link_libraries(al PUBLIC portaudio)
# else ()
#     if (ALLOLIB_USE_DUMMY_AUDIO)
#         target_compile_definitions(al PUBLIC ${PLATFORM_DEFINITION} AL_AUDIO_DUMMY)
#     else()
#         target_compile_definitions(al PUBLIC ${PLATFORM_DEFINITION} AL_AUDIO_RTAUDIO)

#         target_link_libraries(al PUBLIC rtaudio)
#     endif(ALLOLIB_USE_DUMMY_AUDIO)
# endif()

## only used by AppRecorder.cpp? TODO Remove this.
# if(SNDFILE_LIBRARY)
#   target_compile_definitions(al PUBLIC AL_LIBSNDFILE)
# endif(SNDFILE_LIBRARY)


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

