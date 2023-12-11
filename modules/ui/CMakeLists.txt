
## ui and scene modules combined ##
set(headers
  include/al/ui/al_BoundingBox.hpp
  include/al/ui/al_BoundingBoxData.hpp
  include/al/ui/al_Composition.hpp
  include/al/ui/al_ControlGUI.hpp
  include/al/ui/al_Dialog.hpp
  include/al/ui/al_DiscreteParameterValues.hpp
  include/al/ui/al_FileSelector.hpp
  include/al/ui/al_Gnomon.hpp
  include/al/ui/al_HtmlInterfaceServer.hpp
  include/al/ui/al_Parameter.hpp
  include/al/ui/al_ParameterBundle.hpp
  include/al/ui/al_ParameterGUI.hpp
  include/al/ui/al_ParameterMIDI.hpp
  include/al/ui/al_ParameterServer.hpp
  include/al/ui/al_Pickable.hpp
  include/al/ui/al_PickableManager.hpp
  include/al/ui/al_PickableRotateHandle.hpp
  include/al/ui/al_PresetHandler.hpp
  include/al/ui/al_PresetMapper.hpp
  include/al/ui/al_PresetMIDI.hpp
  include/al/ui/al_PresetSequencer.hpp
  include/al/ui/al_SequenceRecorder.hpp
  include/al/ui/al_SequenceServer.hpp

  include/al/scene/al_DistributedScene.hpp
  include/al/scene/al_DynamicScene.hpp
  include/al/scene/al_SynthRecorder.hpp
  include/al/scene/al_PolySynth.hpp
  include/al/scene/al_PositionedVoice.hpp
  include/al/scene/al_SequencerMIDI.hpp
  include/al/scene/al_SynthSequencer.hpp
  include/al/scene/al_SynthVoice.hpp
)

set(sources
  src/ui/al_BoundingBox.cpp
  src/ui/al_BoundingBoxData.cpp
  src/ui/al_Composition.cpp
  src/ui/al_ControlGUI.cpp
  src/ui/al_Dialog.cpp
  src/ui/al_DiscreteParameterValues.cpp
  src/ui/al_FileSelector.cpp
  src/ui/al_Gnomon.cpp
  src/ui/al_HtmlInterfaceServer.cpp
  src/ui/al_ParameterBundle.cpp
  src/ui/al_PresetMIDI.cpp
  src/ui/al_ParameterGUI.cpp
  src/ui/al_ParameterMIDI.cpp
  src/ui/al_PresetSequencer.cpp
  src/ui/al_ParameterServer.cpp
  src/ui/al_SequenceRecorder.cpp
  src/ui/al_SequenceServer.cpp
  src/ui/al_PresetHandler.cpp
  src/ui/al_PresetServer.cpp
  src/ui/al_Parameter.cpp
  src/ui/al_Pickable.cpp
  src/ui/al_PickableManager.cpp
  src/ui/al_PresetMapper.cpp

  src/scene/al_DistributedScene.cpp
  src/scene/al_DynamicScene.cpp
  src/scene/al_SynthRecorder.cpp
  src/scene/al_PolySynth.cpp
  src/scene/al_PositionedVoice.cpp
  src/scene/al_SequencerMIDI.cpp
  src/scene/al_SynthSequencer.cpp
  src/scene/al_SynthVoice.cpp
)

add_library(alui ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(alui PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(alui PUBLIC
  include
)

target_link_libraries(alui PUBLIC
  almath
  algraphics
  alaudio
  alprotocol
  alimgui
  almidi
  alsound
)