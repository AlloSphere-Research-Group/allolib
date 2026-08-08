add_library(alapp ${AL_LIBRARY_TYPE})
add_library(al::app ALIAS alapp)

target_sources(alapp
  PRIVATE
    src/app/al_App.cpp
    src/app/al_AppRecorder.cpp
    src/app/al_DistributedApp.cpp
    src/app/al_FPS.cpp
    src/app/al_AudioDomain.cpp
    src/app/al_ComputationDomain.cpp
    src/app/al_GUIDomain.cpp
    src/app/al_OmniRendererDomain.cpp
    src/app/al_OpenGLGraphicsDomain.cpp
    src/app/al_OSCDomain.cpp
    src/app/al_SimulationDomain.cpp
    src/app/al_StateDistributionDomain.cpp
    src/sphere/al_AlloSphereSpeakerLayout.cpp
    src/sphere/al_SphereUtils.cpp
    src/sphere/al_PerProjection.cpp
    src/sphere/al_Meter.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/app/al_App.hpp
        include/al/app/al_AppRecorder.hpp
        include/al/app/al_DistributedApp.hpp
        include/al/app/al_FPS.hpp
        include/al/app/al_NodeConfiguration.hpp
        include/al/app/al_AudioDomain.hpp
        include/al/app/al_ComputationDomain.hpp
        include/al/app/al_ConsoleDomain.hpp
        include/al/app/al_GUIDomain.hpp
        include/al/app/al_OpenGLGraphicsDomain.hpp
        include/al/app/al_OSCDomain.hpp
        include/al/app/al_SimulationDomain.hpp
        include/al/app/al_OmniRendererDomain.hpp
        include/al/app/al_StateDistributionDomain.hpp
        include/al/sphere/al_AlloSphereSpeakerLayout.hpp
        include/al/sphere/al_SphereUtils.hpp
        include/al/sphere/al_PerProjection.hpp
        include/al/sphere/al_Meter.hpp
)

al_configure_module(alapp EXPORT_NAME app)

target_link_libraries(alapp PUBLIC
  al::audio_device
  al::audio
  al::ui
  al::config
  Gamma
)

al_install_module(alapp)
