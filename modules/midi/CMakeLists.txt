cmake_minimum_required(VERSION 3.5)

if(ALLOLIB_BUILD_SHARED)
    set(RTMIDI_BUILD_STATIC_LIBS OFF  CACHE BOOL "" FORCE)
    set(RTMIDI_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    # set(GAMMA_BUILD_SHARED ON CACHE BOOL "" FORCE)
    # set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
else(ALLOLIB_BUILD_SHARED)
    set(RTMIDI_BUILD_STATIC_LIBS ON  CACHE BOOL "" FORCE)
    set(RTMIDI_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    # set(GAMMA_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    # set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif(ALLOLIB_BUILD_SHARED)

set(RTMIDI_BUILD_TESTING     OFF CACHE BOOL "" FORCE)
set(RTMIDI_TARGETNAME_UNINSTALL RTMIDI_UNINSTALL CACHE STRING "" FORCE)

# # Required for RtAudio 5.2.0
# set (RTAUDIO_STATIC_MSVCRT OFF)

add_subdirectory(external/rtmidi)

set(headers
  include/al/io/al_MIDI.hpp
)

set(sources
  src/io/al_MIDI.cpp
)

add_library(almidi ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(almidi PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(almidi PUBLIC
  include
)

target_link_libraries(almidi PUBLIC
  rtmidi
)