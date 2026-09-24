add_library(alapp ${AL_LIBRARY_TYPE})
add_library(al::app ALIAS alapp)

target_sources(alapp
  PRIVATE
    src/app/al_App.cpp
    src/app/al_AppRecorder.cpp
    src/app/al_DistributedApp.cpp
    src/app/al_FPS.cpp
    src/app/al_Runtime.cpp
    src/app/al_AudioDomain.cpp
    src/app/al_ComputationDomain.cpp
    src/app/al_GUIDomain.cpp
    src/app/al_OmniRendererDomain.cpp
    src/app/al_OpenGLGraphicsDomain.cpp
    src/app/al_OSCDomain.cpp
    src/app/al_SimulationDomain.cpp
    src/app/al_StateDistributionDomain.cpp
    src/app/al_StateSync.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/app/al_App.hpp
        include/al/app/al_AppBuilder.hpp
        include/al/app/al_Module.hpp
        include/al/app/al_AppRecorder.hpp
        include/al/app/al_DistributedApp.hpp
        include/al/app/al_FPS.hpp
        include/al/app/al_Runtime.hpp
        include/al/app/al_KeepAliveDomain.hpp
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
        include/al/app/al_StateSync.hpp
)

al_configure_module(alapp EXPORT_NAME app)

target_link_libraries(alapp PUBLIC
  al::audio_device
  al::audio
  al::scene
  al::ui
  al::config
  al::sphere
  Gamma
)

al_install_module(alapp)
