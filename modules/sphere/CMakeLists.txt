# AlloSphere speakers, per-projection, meter, host utils.
# May move to al_ext later; keep header paths as al/sphere/*.

add_library(alsphere ${AL_LIBRARY_TYPE})
add_library(al::sphere ALIAS alsphere)

target_sources(alsphere
  PRIVATE
    src/sphere/al_AlloSphereSpeakerLayout.cpp
    src/sphere/al_SphereUtils.cpp
    src/sphere/al_PerProjection.cpp
    src/sphere/al_Meter.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/sphere/al_AlloSphereSpeakerLayout.hpp
        include/al/sphere/al_SphereUtils.hpp
        include/al/sphere/al_SphereShell.hpp
        include/al/sphere/al_PerProjection.hpp
        include/al/sphere/al_Meter.hpp
)

al_configure_module(alsphere EXPORT_NAME sphere)

target_link_libraries(alsphere PUBLIC
  al::audio
  al::audio_io
  al::graphics
  al::spatial
  al::system
)

if(AL_WINDOWS)
  target_link_libraries(alsphere PUBLIC ws2_32)
endif()

al_install_module(alsphere)
