option(USE_GLV "" OFF)

# Main Library
set(core_headers
  ${al_path}/include/al/core/app/al_App.hpp
  ${al_path}/include/al/core/app/al_AudioApp.hpp
  ${al_path}/include/al/core/app/al_FPS.hpp
  ${al_path}/include/al/core/app/al_WindowApp.hpp
  ${al_path}/include/al/core/graphics/al_BufferObject.hpp
  ${al_path}/include/al/core/graphics/al_DefaultShaders.hpp
  ${al_path}/include/al/core/graphics/al_EasyFBO.hpp
  ${al_path}/include/al/core/graphics/al_EasyVAO.hpp
  ${al_path}/include/al/core/graphics/al_FBO.hpp
  ${al_path}/include/al/core/graphics/al_GLEW.hpp
  ${al_path}/include/al/core/graphics/al_GLFW.hpp
  ${al_path}/include/al/core/graphics/al_GPUObject.hpp
  ${al_path}/include/al/core/graphics/al_Graphics.hpp
  ${al_path}/include/al/core/graphics/al_Isosurface.hpp
  ${al_path}/include/al/core/graphics/al_Lens.hpp
  ${al_path}/include/al/core/graphics/al_Light.hpp
  ${al_path}/include/al/core/graphics/al_Mesh.hpp
  ${al_path}/include/al/core/graphics/al_OpenGL.hpp
  ${al_path}/include/al/core/graphics/al_RenderManager.hpp
  ${al_path}/include/al/core/graphics/al_Shader.hpp
  ${al_path}/include/al/core/graphics/al_Shapes.hpp
  ${al_path}/include/al/core/graphics/al_Texture.hpp
  ${al_path}/include/al/core/graphics/al_VAO.hpp
  ${al_path}/include/al/core/graphics/al_VAOMesh.hpp
  ${al_path}/include/al/core/graphics/al_Viewpoint.hpp
  ${al_path}/include/al/core/io/al_AudioIO.hpp
  ${al_path}/include/al/core/io/al_AudioIOData.hpp
  ${al_path}/include/al/core/io/al_ControlNav.hpp
  ${al_path}/include/al/core/io/al_CSVReader.hpp
  ${al_path}/include/al/core/io/al_File.hpp
  ${al_path}/include/al/core/io/al_MIDI.hpp
  ${al_path}/include/al/core/io/al_Window.hpp
  ${al_path}/include/al/core/math/al_Constants.hpp
  ${al_path}/include/al/core/math/al_Mat.hpp
  ${al_path}/include/al/core/math/al_Matrix4.hpp
  ${al_path}/include/al/core/math/al_Quat.hpp
  ${al_path}/include/al/core/math/al_StdRandom.hpp
  ${al_path}/include/al/core/math/al_Vec.hpp
  ${al_path}/include/al/core/protocol/al_OSC.hpp
  ${al_path}/include/al/core/sound/al_Ambisonics.hpp
  ${al_path}/include/al/core/sound/al_AudioScene.hpp
  ${al_path}/include/al/core/sound/al_Biquad.hpp
  ${al_path}/include/al/core/sound/al_Crossover.hpp
  ${al_path}/include/al/core/sound/al_Dbap.hpp
  ${al_path}/include/al/core/sound/al_Reverb.hpp
  ${al_path}/include/al/core/sound/al_Speaker.hpp
  ${al_path}/include/al/core/sound/al_StereoPanner.hpp
  ${al_path}/include/al/core/sound/al_Vbap.hpp
  ${al_path}/include/al/core/spatial/al_HashSpace.hpp
  ${al_path}/include/al/core/spatial/al_Pose.hpp
  ${al_path}/include/al/core/system/al_PeriodicThread.hpp
  ${al_path}/include/al/core/system/al_Printing.hpp
  ${al_path}/include/al/core/system/al_Thread.hpp
  ${al_path}/include/al/core/system/al_Time.hpp
  ${al_path}/include/al/core/types/al_Color.hpp
)

set(core_sources
  ${al_path}/src/core/app/al_AudioApp.cpp
  ${al_path}/src/core/app/al_FPS.cpp
  ${al_path}/src/core/app/al_WindowApp.cpp
  ${al_path}/src/core/graphics/al_BufferObject.cpp
  ${al_path}/src/core/graphics/al_DefaultShaders.cpp
  ${al_path}/src/core/graphics/al_EasyFBO.cpp
  ${al_path}/src/core/graphics/al_EasyVAO.cpp
  ${al_path}/src/core/graphics/al_FBO.cpp
  ${al_path}/src/core/graphics/al_GLEW.cpp
  ${al_path}/src/core/graphics/al_GLFW.cpp
  ${al_path}/src/core/graphics/al_GPUObject.cpp
  ${al_path}/src/core/graphics/al_Graphics.cpp
  ${al_path}/src/core/graphics/al_Isosurface.cpp
  ${al_path}/src/core/graphics/al_Lens.cpp
  ${al_path}/src/core/graphics/al_Light.cpp
  ${al_path}/src/core/graphics/al_Mesh.cpp
  ${al_path}/src/core/graphics/al_OpenGL.cpp
  ${al_path}/src/core/graphics/al_RenderManager.cpp
  ${al_path}/src/core/graphics/al_Shader.cpp
  ${al_path}/src/core/graphics/al_Shapes.cpp
  ${al_path}/src/core/graphics/al_Texture.cpp
  ${al_path}/src/core/graphics/al_VAO.cpp
  ${al_path}/src/core/graphics/al_VAOMesh.cpp
  ${al_path}/src/core/graphics/al_Viewpoint.cpp
  ${al_path}/src/core/io/al_AudioIO.cpp
  ${al_path}/src/core/io/al_AudioIOData.cpp
  ${al_path}/src/core/io/al_ControlNav.cpp
  ${al_path}/src/core/io/al_CSVReader.cpp
  ${al_path}/src/core/io/al_File.cpp
  ${al_path}/src/core/io/al_MIDI.cpp
  ${al_path}/src/core/io/al_Window.cpp
  ${al_path}/src/core/io/al_WindowGLFW.cpp
  ${al_path}/src/core/math/al_StdRandom.cpp
  ${al_path}/src/core/protocol/al_OSC.cpp
  ${al_path}/src/core/sound/al_Ambisonics.cpp
  ${al_path}/src/core/sound/al_AudioScene.cpp
  ${al_path}/src/core/sound/al_Biquad.cpp
  ${al_path}/src/core/sound/al_Dbap.cpp
  ${al_path}/src/core/sound/al_Vbap.cpp
  ${al_path}/src/core/spatial/al_HashSpace.cpp
  ${al_path}/src/core/spatial/al_Pose.cpp
  ${al_path}/src/core/system/al_PeriodicThread.cpp
  ${al_path}/src/core/system/al_Printing.cpp
  ${al_path}/src/core/system/al_ThreadNative.cpp
  ${al_path}/src/core/system/al_Time.cpp
  ${al_path}/src/core/types/al_Color.cpp
)

set(util_headers
  ${al_path}/include/al/util/al_Array.h
  ${al_path}/include/al/util/al_Array.hpp
  ${al_path}/include/al/util/imgui/imgui_impl_glfw_gl3.h
  ${al_path}/include/al/util/ui/al_Composition.hpp
  ${al_path}/include/al/util/ui/al_Parameter.hpp
  # ${al_path}/include/al/util/ui/al_ParameterMIDI.hpp
  ${al_path}/include/al/util/ui/al_Preset.hpp
  ${al_path}/include/al/util/ui/al_PresetMapper.hpp
  # ${al_path}/include/al/util/ui/al_PresetMIDI.hpp
  ${al_path}/include/al/util/ui/al_PresetSequencer.hpp
  ${al_path}/include/al/util/ui/al_SequenceRecorder.hpp
  ${al_path}/include/al/util/ui/al_SynthSequencer.hpp
)

set(util_sources
  ${al_path}/src/util/al_Array_C.c
  ${al_path}/src/util/al_Array.cpp
  ${al_path}/src/util/imgui/imgui_impl_glfw_gl3.cpp
  ${al_path}/src/util/ui/al_Composition.cpp
  ${al_path}/src/util/ui/al_Parameter.cpp
  ${al_path}/src/util/ui/al_Preset.cpp
  ${al_path}/src/util/ui/al_PresetMapper.cpp
  # ${al_path}/src/util/ui/al_PresetMIDI.cpp
  ${al_path}/src/util/ui/al_PresetSequencer.cpp
  ${al_path}/src/util/ui/al_SequenceRecorder.cpp
  ${al_path}/src/util/ui/al_SynthSequencer.cpp
)

set(al_headers
  ${core_headers}
  ${util_headers}
)

set(al_sources
  ${core_sources}
  ${util_sources}
)

# OPTIONAL ---------------------------------------------------------------------

set(glv_headers
  include/al/glv/al_GLV.hpp
  include/al/glv/glv.h
  include/al/glv/glv_behavior.h
  include/al/glv/glv_buttons.h
  include/al/glv/glv_conf.h
  include/al/glv/glv_core.h
  include/al/glv/glv_font.h
  include/al/glv/glv_layout.h
  include/al/glv/glv_model.h
  include/al/glv/glv_notification.h
  include/al/glv/glv_rect.h
  include/al/glv/glv_sliders.h
  include/al/glv/glv_textview.h
  include/al/glv/glv_util.h
  include/al/glv/glv_widget.h
)

set(glv_sources
  src/glv/al_GLV_draw.cpp
  src/glv/al_GLV_wrapper.cpp
  src/glv/glv_buttons.cpp
  src/glv/glv_core.cpp
  src/glv/glv_font.cpp
  src/glv/glv_glv.cpp
  src/glv/glv_inputdevice.cpp
  src/glv/glv_layout.cpp
  src/glv/glv_model.cpp
  src/glv/glv_notification.cpp
  src/glv/glv_sliders.cpp
  src/glv/glv_textview.cpp
  src/glv/glv_view.cpp
  src/glv/glv_widget.cpp
)

if (USE_GLV)
  list(APPEND al_headers ${glv_headers})
  list(APPEND al_sources ${glv_sources})
endif()
