if(ALLOLIB_BUILD_SHARED)
    set(RTAUDIO_BUILD_STATIC_LIBS OFF  CACHE BOOL "" FORCE)
    set(RTAUDIO_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
else(ALLOLIB_BUILD_SHARED)
    set(RTAUDIO_BUILD_STATIC_LIBS ON  CACHE BOOL "" FORCE)
    set(RTAUDIO_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif(ALLOLIB_BUILD_SHARED)

set(RTAUDIO_BUILD_TESTING     OFF CACHE BOOL "" FORCE)
set(RTAUDIO_TARGETNAME_UNINSTALL RTAUDIO_UNINSTALL CACHE STRING "" FORCE)

# Required for RtAudio 5.2.0
set (RTAUDIO_STATIC_MSVCRT OFF)

add_subdirectory(external/rtaudio)

## alaudiobackend_rtaudio

set(headers

)

set(sources
  src/io/al_AudioBackend.cpp
  # src/io/al_AudioIOData.cpp
)

add_library(alaudiobackend ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(alaudiobackend PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

# target_include_directories(alaudiobackend PUBLIC
# )

target_link_libraries(alaudiobackend PUBLIC
  rtaudio
  alaudio
)