## dependency dr_libs ##
add_subdirectory(external/dr_libs)

## alsound ##
set(headers
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

set(sources
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
)

add_library(alsound ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(alsound PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(alsound PUBLIC
  include
)

target_link_libraries(alsound PUBLIC
  almath
  alspatial
  alaudio
  dr_libs
)