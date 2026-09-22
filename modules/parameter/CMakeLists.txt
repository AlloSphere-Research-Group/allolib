# Parameter / composition / future ECS signal layer.
# Legacy headers remain under al/ui/...; new API under al/parameter/.

add_library(alparameter ${AL_LIBRARY_TYPE})
add_library(al::parameter ALIAS alparameter)

target_sources(alparameter
  PRIVATE
    src/parameter/al_Entity.cpp
    src/parameter/al_KeyframeTrack.cpp
    src/parameter/al_ParamSet.cpp
    src/parameter/al_ParamState.cpp
    src/parameter/al_Scene.cpp
    src/ui/al_DiscreteParameterValues.cpp
    src/ui/al_ParamCodec.cpp
    src/ui/al_Parameter.cpp
    src/ui/al_ParameterBundle.cpp
    src/ui/al_ParameterMIDI.cpp
    src/ui/al_ParameterServer.cpp
    src/ui/al_PresetHandler.cpp
    src/ui/al_PresetMapper.cpp
    src/ui/al_PresetMIDI.cpp
    src/ui/al_PresetSequencer.cpp
    src/ui/al_PresetServer.cpp
    src/ui/al_SequenceRecorder.cpp
    src/ui/al_SequenceServer.cpp
    src/ui/al_Composition.cpp
    src/ui/al_HtmlInterfaceServer.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/parameter/al_Entity.hpp
        include/al/parameter/al_KeyframeTrack.hpp
        include/al/parameter/al_ParamSet.hpp
        include/al/parameter/al_ParamState.hpp
        include/al/parameter/al_Scene.hpp
        include/al/parameter/al_Signal.hpp
        include/al/ui/al_DiscreteParameterValues.hpp
        include/al/ui/al_ParamCodec.hpp
        include/al/ui/al_ParamNumeric.hpp
        include/al/ui/al_Parameter.hpp
        include/al/ui/al_ParameterBundle.hpp
        include/al/ui/al_ParameterMIDI.hpp
        include/al/ui/al_ParameterServer.hpp
        include/al/ui/al_PresetHandler.hpp
        include/al/ui/al_PresetMapper.hpp
        include/al/ui/al_PresetMIDI.hpp
        include/al/ui/al_PresetSequencer.hpp
        include/al/ui/al_PresetServer.hpp
        include/al/ui/al_SequenceRecorder.hpp
        include/al/ui/al_SequenceServer.hpp
        include/al/ui/al_Composition.hpp
        include/al/ui/al_HtmlInterfaceServer.hpp
)

al_configure_module(alparameter EXPORT_NAME parameter)

target_link_libraries(alparameter PUBLIC
  al::math
  al::spatial
  al::types
  al::system
  al::io
  al::protocol
  al::midi
)

al_install_module(alparameter)
