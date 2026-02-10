
add_subdirectory(external/cpptoml)

set(headers
  include/al/io/al_PersistentConfig.hpp
  include/al/io/al_Toml.hpp
)

set(sources
  src/io/al_PersistentConfig.cpp
  src/io/al_Toml.cpp
)

add_library(alconfig ${AL_LIBRARY_TYPE} ${headers} ${sources})

if(AL_WINDOWS)
  target_compile_definitions(alconfig PUBLIC NOMINMAX)
endif(AL_WINDOWS)

set_target_properties(alconfig PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(alconfig PUBLIC
  include
)

target_link_libraries(alconfig PUBLIC
  alio
  alui
  cpptoml
)