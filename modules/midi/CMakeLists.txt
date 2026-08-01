if(ALLOLIB_BUILD_SHARED)
  set(RTMIDI_BUILD_STATIC_LIBS OFF CACHE BOOL "" FORCE)
  set(RTMIDI_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
else()
  set(RTMIDI_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
  set(RTMIDI_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif()

set(RTMIDI_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(RTMIDI_TARGETNAME_UNINSTALL RTMIDI_UNINSTALL CACHE STRING "" FORCE)

add_subdirectory(external/rtmidi)

add_library(almidi ${AL_LIBRARY_TYPE})
add_library(al::midi ALIAS almidi)

target_sources(almidi
  PRIVATE
    src/io/al_MIDI.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/io/al_MIDI.hpp
)

al_configure_module(almidi EXPORT_NAME midi)

if(AL_WINDOWS)
  target_compile_definitions(almidi PUBLIC NOMINMAX)
endif()

target_link_libraries(almidi PUBLIC
  rtmidi
)

al_install_module(almidi)
