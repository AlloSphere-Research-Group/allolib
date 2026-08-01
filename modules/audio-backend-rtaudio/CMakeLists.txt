if(ALLOLIB_BUILD_SHARED)
    set(RTAUDIO_BUILD_STATIC_LIBS OFF  CACHE BOOL "" FORCE)
    set(RTAUDIO_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
else()
    set(RTAUDIO_BUILD_STATIC_LIBS ON  CACHE BOOL "" FORCE)
    set(RTAUDIO_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif()

set(RTAUDIO_BUILD_TESTING     OFF CACHE BOOL "" FORCE)
set(RTAUDIO_TARGETNAME_UNINSTALL RTAUDIO_UNINSTALL CACHE STRING "" FORCE)

# Required for RtAudio 5.2.0
set(RTAUDIO_STATIC_MSVCRT OFF)

add_subdirectory(external/rtaudio)

add_library(alaudiobackend ${AL_LIBRARY_TYPE})
add_library(al::audio_backend ALIAS alaudiobackend)

target_sources(alaudiobackend
  PRIVATE
    src/io/al_AudioBackend.cpp
)

al_configure_module(alaudiobackend EXPORT_NAME audio_backend)

if(AL_WINDOWS)
  target_compile_definitions(alaudiobackend PUBLIC NOMINMAX)
endif()

target_link_libraries(alaudiobackend PUBLIC
  rtaudio
  al::audio
)

al_install_module(alaudiobackend)
