
set(headers
  include/al/system/al_PeriodicThread.hpp
  include/al/system/al_Printing.hpp
  include/al/system/al_Thread.hpp
  include/al/system/al_Time.hpp
)

set(sources
  src/system/al_PeriodicThread.cpp
  src/system/al_Printing.cpp
  src/system/al_ThreadNative.cpp
  src/system/al_Time.cpp

)

add_library(alsystem ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(alsystem PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(alsystem PUBLIC
  include
)

target_link_libraries(alsystem PUBLIC
  almath
)

target_compile_definitions(alsystem PUBLIC ${PLATFORM_DEFINITION})