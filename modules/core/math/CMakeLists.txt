
# why did allolib not list all headers?
set(headers
  include/al/math/al_Constants.hpp
  include/al/math/al_Mat.hpp
  include/al/math/al_Matrix4.hpp
  include/al/math/al_Quat.hpp
  include/al/math/al_StdRandom.hpp
  include/al/math/al_Vec.hpp
)

set(sources
  src/math/al_StdRandom.cpp
)

add_library(almath ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(almath PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(almath PUBLIC
  include
)