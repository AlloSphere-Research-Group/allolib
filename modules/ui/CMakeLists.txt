## Graphics UI + imgui bindings. Scene still lives here until a later split.

add_library(alui ${AL_LIBRARY_TYPE})
add_library(al::ui ALIAS alui)

target_sources(alui
  PRIVATE
    src/ui/al_BoundingBox.cpp
    src/ui/al_BoundingBoxData.cpp
    src/ui/al_ControlGUI.cpp
    src/ui/al_Dialog.cpp
    src/ui/al_FileSelector.cpp
    src/ui/al_Gnomon.cpp
    src/ui/al_ParameterGUI.cpp
    src/ui/al_Pickable.cpp
    src/ui/al_PickableManager.cpp
    src/scene/al_DistributedScene.cpp
    src/scene/al_DynamicScene.cpp
    src/scene/al_SynthRecorder.cpp
    src/scene/al_PolySynth.cpp
    src/scene/al_PositionedVoice.cpp
    src/scene/al_SequencerMIDI.cpp
    src/scene/al_SynthSequencer.cpp
    src/scene/al_SynthVoice.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/ui/al_BoundingBox.hpp
        include/al/ui/al_BoundingBoxData.hpp
        include/al/ui/al_ControlGUI.hpp
        include/al/ui/al_Dialog.hpp
        include/al/ui/al_FileSelector.hpp
        include/al/ui/al_Gnomon.hpp
        include/al/ui/al_ParameterGUI.hpp
        include/al/ui/al_Pickable.hpp
        include/al/ui/al_PickableManager.hpp
        include/al/ui/al_PickableRotateHandle.hpp
        include/al/scene/al_DistributedScene.hpp
        include/al/scene/al_DynamicScene.hpp
        include/al/scene/al_SynthRecorder.hpp
        include/al/scene/al_PolySynth.hpp
        include/al/scene/al_PositionedVoice.hpp
        include/al/scene/al_SequencerMIDI.hpp
        include/al/scene/al_SynthSequencer.hpp
        include/al/scene/al_SynthVoice.hpp
)

al_configure_module(alui EXPORT_NAME ui)

target_link_libraries(alui PUBLIC
  al::parameter
  al::math
  al::graphics
  al::audio
  al::imgui
)

al_install_module(alui)
