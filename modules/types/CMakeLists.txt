
# add_subdirectory(external/oscpack)

# find_package(oscpack REQUIRED)

set(headers
  include/al/types/al_Color.hpp
  include/al/types/al_VariantValue.hpp
)

set(sources
  src/types/al_Color.cpp
  src/types/al_VariantValue.cpp
)

add_library(altypes ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(altypes PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(altypes PUBLIC
  include
)

target_link_libraries(altypes PUBLIC
  almath
)