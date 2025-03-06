cmake_minimum_required(VERSION 3.5)

if(ALLOLIB_BUILD_SHARED)
    set(RTMIDI_BUILD_STATIC_LIBS OFF  CACHE BOOL "" FORCE)
    set(RTMIDI_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    # set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
else(ALLOLIB_BUILD_SHARED)
    set(RTMIDI_BUILD_STATIC_LIBS ON  CACHE BOOL "" FORCE)
    set(RTMIDI_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    # set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif(ALLOLIB_BUILD_SHARED)

set(RTMIDI_BUILD_TESTING     OFF CACHE BOOL "" FORCE)
set(RTMIDI_TARGETNAME_UNINSTALL RTMIDI_UNINSTALL CACHE STRING "" FORCE)

# # Required for RtAudio 5.2.0
# set (RTAUDIO_STATIC_MSVCRT OFF)

if(APPLE)
    # Disable JACK for RtMidi on macOS since CoreMIDI is preferred
    set(RTMIDI_API_JACK OFF CACHE BOOL "Build JACK API support" FORCE)
    set(RTMIDI_API_CORE ON CACHE BOOL "Build CoreMIDI API support" FORCE)

    # Add CoreMIDI definition explicitly
    # add_definitions(-D__MACOSX_CORE__)
    
    # Remove UNIX_JACK definition if it exists
    # remove_definitions(-D__UNIX_JACK__)
endif()

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

# Add compile definitions to the target specifically
# if(APPLE)
#     target_compile_definitions(almidi PUBLIC
#         __MACOSX_CORE__
#     )
# endif()

target_link_libraries(almidi PUBLIC
  rtmidi
)


# On macOS, ensure CoreMIDI frameworks are linked
# if(APPLE)
#     find_library(COREMIDI_FRAMEWORK CoreMIDI)
#     find_library(COREAUDIO_FRAMEWORK CoreAudio)
#     find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
#     target_link_libraries(almidi PUBLIC
#         ${COREMIDI_FRAMEWORK}
#         ${COREAUDIO_FRAMEWORK}
#         ${COREFOUNDATION_FRAMEWORK}
#     )
# endif()