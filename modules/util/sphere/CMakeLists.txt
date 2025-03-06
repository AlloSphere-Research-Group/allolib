

set(headers
  include/al/sphere/al_AlloSphereSpeakerLayout.hpp
  include/al/sphere/al_SphereUtils.hpp
  include/al/sphere/al_PerProjection.hpp
  include/al/sphere/al_Meter.hpp
)

set(sources
  src/sphere/al_AlloSphereSpeakerLayout.cpp
  src/sphere/al_SphereUtils.cpp
  src/sphere/al_PerProjection.cpp
  src/sphere/al_Meter.cpp
)

add_library(alsphere ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(alsphere PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(alsphere PUBLIC
  include
)

target_link_libraries(alsphere PUBLIC
  alaudiobackend
  almath
  alui
  alconfig
  alapp
  Gamma
)