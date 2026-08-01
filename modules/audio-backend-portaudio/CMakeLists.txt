## alaudiobackend_portaudio

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")
find_package(Portaudio REQUIRED)

set(headers

)

set(sources
  src/io/al_AudioBackend.cpp
)

add_library(alaudiobackend ${AL_LIBRARY_TYPE} ${headers} ${sources})
add_library(al::audio_backend ALIAS alaudiobackend)

set_target_properties(alaudiobackend PROPERTIES
  EXPORT_NAME audio_backend
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(alaudiobackend PUBLIC
  ${PORTAUDIO_INCLUDE_DIR}
)

target_link_libraries(alaudiobackend PUBLIC
  alaudio
  ${PORTAUDIO_LIBRARY}
)