list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")
find_package(Portaudio REQUIRED)

add_library(alaudiobackend ${AL_LIBRARY_TYPE})
add_library(al::audio_backend ALIAS alaudiobackend)

target_sources(alaudiobackend
  PRIVATE
    src/io/al_AudioBackend.cpp
)

al_configure_module(alaudiobackend EXPORT_NAME audio_backend)

target_include_directories(alaudiobackend PUBLIC
  ${PORTAUDIO_INCLUDE_DIR}
)

target_link_libraries(alaudiobackend PUBLIC
  al::audio_io
  ${PORTAUDIO_LIBRARY}
)

al_install_module(alaudiobackend)
