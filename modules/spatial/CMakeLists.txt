
set(headers
  include/al/spatial/al_HashSpace.hpp
  include/al/spatial/al_Pose.hpp
  include/al/spatial/al_Curve.hpp
)

set(sources
  src/spatial/al_HashSpace.cpp
  src/spatial/al_Pose.cpp
)

add_library(alspatial ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(alspatial PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(alspatial PUBLIC
  include
)

target_link_libraries(alspatial PUBLIC
  almath
)