

set(headers
#   include/al/app/al_App.hpp
#   include/al/app/al_AppRecorder.hpp
#   include/al/app/al_DistributedApp.hpp
#   include/al/app/al_FPS.hpp
#   include/al/app/al_NodeConfiguration.hpp

  include/al/domain/al_AudioDomain.hpp
  include/al/domain/al_ComputationDomain.hpp
  include/al/domain/al_ConsoleDomain.hpp
  include/al/domain/al_GUIDomain.hpp
  include/al/domain/al_OpenGLGraphicsDomain.hpp
  include/al/domain/al_OSCDomain.hpp
  include/al/domain/al_SimulationDomain.hpp
  include/al/domain/al_OmniRendererDomain.hpp
  include/al/domain/al_StateDistributionDomain.hpp

  # include/al/sphere/al_AlloSphereSpeakerLayout.hpp
  # include/al/sphere/al_SphereUtils.hpp
  # include/al/sphere/al_PerProjection.hpp
  # include/al/sphere/al_Meter.hpp
)

set(sources
#   src/app/al_App.cpp
#   src/app/al_AppRecorder.cpp
#   src/app/al_DistributedApp.cpp
#   src/app/al_FPS.cpp

  src/core/al_ComputationDomain.cpp
  src/core/al_SimulationDomain.cpp
  src/system/al_AudioDomain.cpp
  src/system/al_GUIDomain.cpp
  src/system/al_OmniRendererDomain.cpp
  src/system/al_OpenGLGraphicsDomain.cpp
  src/distribution/al_OSCDomain.cpp
  src/distribution/al_StateDistributionDomain.cpp

  # src/sphere/al_AlloSphereSpeakerLayout.cpp
  # src/sphere/al_SphereUtils.cpp
  # src/sphere/al_PerProjection.cpp
  # src/sphere/al_Meter.cpp
)

add_library(aldomain ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(aldomain PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(aldomain PUBLIC
  include
)

target_link_libraries(aldomain PUBLIC
  alaudiobackend
  algraphics
  alui
  alconfig
  alsphere
  Gamma
)