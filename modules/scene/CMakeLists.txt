# Synth voices, spatialized scene, sequencers.
# Depends on parameter + audio (+ graphics today: SynthVoice/PolySynth include al_Graphics).
# Does not depend on imgui or App.

add_library(alscene ${AL_LIBRARY_TYPE})
add_library(al::scene ALIAS alscene)

target_sources(alscene
  PRIVATE
    src/scene/al_DistributedScene.cpp
    src/scene/al_DynamicScene.cpp
    src/scene/al_PolySynth.cpp
    src/scene/al_PositionedVoice.cpp
    src/scene/al_SequencerMIDI.cpp
    src/scene/al_SynthRecorder.cpp
    src/scene/al_SynthSequencer.cpp
    src/scene/al_SynthVoice.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/scene/al_DistributedScene.hpp
        include/al/scene/al_DynamicScene.hpp
        include/al/scene/al_PolySynth.hpp
        include/al/scene/al_PositionedVoice.hpp
        include/al/scene/al_SequencerMIDI.hpp
        include/al/scene/al_SynthRecorder.hpp
        include/al/scene/al_SynthSequencer.hpp
        include/al/scene/al_SynthVoice.hpp
)

al_configure_module(alscene EXPORT_NAME scene)

target_link_libraries(alscene PUBLIC
  al::parameter
  al::audio
  al::graphics
  al::spatial
  al::midi
  al::protocol
)

al_install_module(alscene)
