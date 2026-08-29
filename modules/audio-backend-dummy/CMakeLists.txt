add_library(alaudiobackend ${AL_LIBRARY_TYPE})
add_library(al::audio_backend ALIAS alaudiobackend)

target_sources(alaudiobackend
  PRIVATE
    src/io/al_AudioBackend.cpp
)

al_configure_module(alaudiobackend EXPORT_NAME audio_backend)

target_link_libraries(alaudiobackend PUBLIC
  al::audio_io
)

al_install_module(alaudiobackend)
