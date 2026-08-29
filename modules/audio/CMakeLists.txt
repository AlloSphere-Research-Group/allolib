## dependency dr_libs ##
add_subdirectory(external/dr_libs)

add_library(alaudio ${AL_LIBRARY_TYPE})
add_library(al::audio ALIAS alaudio)

target_sources(alaudio
  PRIVATE
    src/sound/al_Ambisonics.cpp
    src/sound/al_Biquad.cpp
    src/sound/al_Dbap.cpp
    src/sound/al_DownMixer.cpp
    src/sound/al_Lbap.cpp
    src/sound/al_Spatializer.cpp
    src/sound/al_Speaker.cpp
    src/sound/al_SpeakerAdjustment.cpp
    src/sound/al_StereoPanner.cpp
    src/sound/al_Vbap.cpp
    src/sound/al_SoundFile.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/sound/al_Ambisonics.hpp
        include/al/sound/al_Biquad.hpp
        include/al/sound/al_Crossover.hpp
        include/al/sound/al_Dbap.hpp
        include/al/sound/al_DownMixer.hpp
        include/al/sound/al_Lbap.hpp
        include/al/sound/al_Reverb.hpp
        include/al/sound/al_Spatializer.hpp
        include/al/sound/al_Speaker.hpp
        include/al/sound/al_SpeakerAdjustment.hpp
        include/al/sound/al_StereoPanner.hpp
        include/al/sound/al_Vbap.hpp
        include/al/sound/al_SoundFile.hpp
)

al_configure_module(alaudio EXPORT_NAME audio)

# Spatializers render into AudioIOData; no dependency on a concrete backend.
target_link_libraries(alaudio PUBLIC
  al::audio_io
  al::math
  al::spatial
  dr_libs
)

al_install_module(alaudio)
