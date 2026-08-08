add_library(alaudio_io ${AL_LIBRARY_TYPE})
add_library(al::audio_io ALIAS alaudio_io)

target_sources(alaudio_io
  PRIVATE
    src/io/al_AudioIO.cpp
    src/io/al_AudioIOData.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/io/al_AudioIO.hpp
        include/al/io/al_AudioIOData.hpp
)

al_configure_module(alaudio_io EXPORT_NAME audio_io)
al_install_module(alaudio_io)
