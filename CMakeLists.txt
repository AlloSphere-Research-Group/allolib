cmake_minimum_required(VERSION 3.0)
project(allolib)

MACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD MSG)
     STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}"
     "${CMAKE_BINARY_DIR}" insource)
     GET_FILENAME_COMPONENT(PARENTDIR ${CMAKE_SOURCE_DIR} PATH)
     STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}"
     "${PARENTDIR}" insourcesubdir)
    IF(insource OR insourcesubdir)
        MESSAGE(FATAL_ERROR "${MSG}")
    ENDIF(insource OR insourcesubdir)
ENDMACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD)

MACRO_ENSURE_OUT_OF_SOURCE_BUILD(
    "${CMAKE_PROJECT_NAME} requires an out of source build. Make a directory and run cmake there."
)

option(AL_VERBOSE_OUTPUT "" OFF)
option(ALLOLIB_BUILD_EXAMPLES "" OFF)
option(TRAVIS_BUILD "" OFF)
option(APPVEYOR_BUILD "" OFF)
option(ALLOLIB_BUILD_TESTS "" OFF)
option(ALLOLIB_USE_PORTAUDIO "Use PortAudio instead of RtAudio" OFF)
option(ALLOLIB_USE_DUMMY_AUDIO "Use Dummy Audio I/O" OFF)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(AL_MACOS 1 CACHE BOOL "Building on OS X")
  set(PLATFORM_DEFINITION -DAL_OSX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(AL_LINUX 1 CACHE BOOL "Building on Linux")
  set(PLATFORM_DEFINITION -DAL_LINUX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(AL_WINDOWS 1 CACHE BOOL "Building on Windows")
  set(PLATFORM_DEFINITION -DAL_WINDOWS)
  if (NOT ${CMAKE_GENERATOR_PLATFORM} STREQUAL "x64")
  message("Warning! Not building for Win64 : ${CMAKE_GENERATOR_PLATFORM}")
  endif(NOT ${CMAKE_GENERATOR_PLATFORM} STREQUAL "x64")
else ()
  message(FATAL_ERROR "system platform not defined")
endif ()

if(TRAVIS_BUILD) 
  add_definitions(-DTRAVIS_BUILD)
endif(TRAVIS_BUILD)

find_package(OpenGL REQUIRED)
add_subdirectory(external)

set(headers
  include/al/app/al_App.hpp
  include/al/app/al_AppRecorder.hpp
  include/al/app/al_DistributedApp.hpp
  include/al/app/al_FPS.hpp
  include/al/app/al_NodeConfiguration.hpp

  include/al/app/al_AudioDomain.hpp
  include/al/app/al_ComputationDomain.hpp
  include/al/app/al_ConsoleDomain.hpp
  include/al/app/al_OpenGLGraphicsDomain.hpp
  include/al/app/al_OSCDomain.hpp
  include/al/app/al_SimulationDomain.hpp
  include/al/app/al_OmniRendererDomain.hpp
  include/al/app/al_StateDistributionDomain.hpp

  include/al/graphics/al_BufferObject.hpp
  include/al/graphics/al_DefaultShaders.hpp
  include/al/graphics/al_DefaultShaderString.hpp
  include/al/graphics/al_EasyFBO.hpp
  include/al/graphics/al_EasyVAO.hpp
  include/al/graphics/al_FBO.hpp
  include/al/graphics/al_Font.hpp
  include/al/graphics/al_GPUObject.hpp
  include/al/graphics/al_Graphics.hpp
  include/al/graphics/al_Image.hpp
  include/al/graphics/al_Isosurface.hpp
  include/al/graphics/al_Lens.hpp
  include/al/graphics/al_Light.hpp
  include/al/graphics/al_Mesh.hpp
  include/al/graphics/al_OpenGL.hpp
  include/al/graphics/al_RenderManager.hpp
  include/al/graphics/al_Shader.hpp
  include/al/graphics/al_Shapes.hpp
  include/al/graphics/al_Texture.hpp
  include/al/graphics/al_VAO.hpp
  include/al/graphics/al_VAOMesh.hpp
  include/al/graphics/al_Viewpoint.hpp

  include/al/io/al_Arduino.hpp
  include/al/io/al_AudioIO.hpp
  include/al/io/al_AudioIOData.hpp
  include/al/io/al_ControlNav.hpp
  include/al/io/al_CSVReader.hpp
  include/al/io/al_File.hpp
  include/al/io/al_Imgui.hpp
  include/al/io/al_MIDI.hpp
  include/al/io/al_PersistentConfig.hpp
  include/al/io/al_Socket.hpp
  include/al/io/al_Toml.hpp
  include/al/io/al_Window.hpp

  include/al/math/al_Constants.hpp
  include/al/math/al_Mat.hpp
  include/al/math/al_Matrix4.hpp
  include/al/math/al_Quat.hpp
  include/al/math/al_StdRandom.hpp
  include/al/math/al_Vec.hpp

  include/al/protocol/al_OSC.hpp
  include/al/protocol/al_CommandConnection.hpp

  include/al/scene/al_DistributedScene.hpp
  include/al/scene/al_DynamicScene.hpp
  include/al/scene/al_SynthRecorder.hpp
  include/al/scene/al_PolySynth.hpp
  include/al/scene/al_SequencerMIDI.hpp
  include/al/scene/al_SynthSequencer.hpp

  include/al/sound/al_Ambisonics.hpp
  include/al/sound/al_Biquad.hpp
  include/al/sound/al_Crossover.hpp
  include/al/sound/al_Dbap.hpp
  include/al/sound/al_Lbap.hpp
  include/al/sound/al_Reverb.hpp
  include/al/sound/al_Spatializer.hpp
  include/al/sound/al_Speaker.hpp
  include/al/sound/al_SpeakerAdjustment.hpp
  include/al/sound/al_StereoPanner.hpp
  include/al/sound/al_Vbap.hpp
  include/al/sound/al_SoundFile.hpp

  include/al/spatial/al_HashSpace.hpp
  include/al/spatial/al_Pose.hpp
  include/al/spatial/al_Curve.hpp

  include/al/sphere/al_SphereUtils.hpp
  include/al/sphere/al_PerProjection.hpp

  include/al/system/al_PeriodicThread.hpp
  include/al/system/al_Printing.hpp
  include/al/system/al_Thread.hpp
  include/al/system/al_Time.hpp

  include/al/types/al_Color.hpp
  include/al/types/al_VariantValue.hpp

  include/al/ui/al_BoundingBox.hpp
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
)

set(sources
  src/app/al_App.cpp
  src/app/al_AppRecorder.cpp
  src/app/al_DistributedApp.cpp
  src/app/al_FPS.cpp

  src/app/al_AudioDomain.cpp
  src/app/al_ComputationDomain.cpp
  src/app/al_OmniRendererDomain.cpp
  src/app/al_OpenGLGraphicsDomain.cpp
  src/app/al_OSCDomain.cpp
  src/app/al_SimulationDomain.cpp
  src/app/al_StateDistributionDomain.cpp

  src/graphics/al_BufferObject.cpp
  src/graphics/al_DefaultShaders.cpp
  src/graphics/al_DefaultShaderString.cpp
  src/graphics/al_EasyFBO.cpp
  src/graphics/al_EasyVAO.cpp
  src/graphics/al_FBO.cpp
  src/graphics/al_Font.cpp
  src/graphics/al_GPUObject.cpp
  src/graphics/al_Graphics.cpp
  src/graphics/al_Image.cpp
  src/graphics/al_Isosurface.cpp
  src/graphics/al_Lens.cpp
  src/graphics/al_Light.cpp
  src/graphics/al_Mesh.cpp
  src/graphics/al_OpenGL.cpp
  src/graphics/al_RenderManager.cpp
  src/graphics/al_Shader.cpp
  src/graphics/al_Shapes.cpp
  src/graphics/al_Texture.cpp
  src/graphics/al_VAO.cpp
  src/graphics/al_VAOMesh.cpp
  src/graphics/al_Viewpoint.cpp

  src/graphics/al_stb_image.cpp
  src/graphics/al_stb_font.cpp

  src/io/al_Arduino.cpp
  src/io/al_AudioIO.cpp
  src/io/al_AudioIOData.cpp
  src/io/al_ControlNav.cpp
  src/io/al_CSVReader.cpp
  src/io/al_File.cpp
  src/io/al_Imgui.cpp
  src/io/al_MIDI.cpp
  src/io/al_PersistentConfig.cpp
  src/io/al_Socket.cpp
  src/io/al_Toml.cpp
  src/io/al_Window.cpp

  src/io/al_WindowGLFW.cpp
  src/io/al_imgui_impl.cpp

  src/math/al_StdRandom.cpp

  src/protocol/al_OSC.cpp
  src/protocol/al_CommandConnection.cpp

  src/scene/al_DistributedScene.cpp
  src/scene/al_DynamicScene.cpp
  src/scene/al_SynthRecorder.cpp
  src/scene/al_PolySynth.cpp
  src/scene/al_SequencerMIDI.cpp
  src/scene/al_SynthSequencer.cpp

  src/sound/al_Ambisonics.cpp
  src/sound/al_Biquad.cpp
  src/sound/al_Dbap.cpp
  src/sound/al_Lbap.cpp
  src/sound/al_Spatializer.cpp
  src/sound/al_Speaker.cpp
  src/sound/al_SpeakerAdjustment.cpp
  src/sound/al_StereoPanner.cpp
  src/sound/al_Vbap.cpp
  src/sound/al_SoundFile.cpp

  src/spatial/al_HashSpace.cpp
  src/spatial/al_Pose.cpp

  src/sphere/al_AlloSphereSpeakerLayout.cpp
  src/sphere/al_SphereUtils.cpp
  src/sphere/al_PerProjection.cpp

  src/system/al_PeriodicThread.cpp
  src/system/al_Printing.cpp
  src/system/al_ThreadNative.cpp
  src/system/al_Time.cpp

  src/types/al_Color.cpp
  src/types/al_VariantValue.cpp

  src/ui/al_BoundingBox.cpp
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

)

add_library(al STATIC ${headers} ${sources})

set_target_properties(al PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

if (AL_WINDOWS)
  target_compile_definitions(al PUBLIC NOMINMAX)
else()
    target_compile_options(al PRIVATE "-Wall")
endif (AL_WINDOWS)

if (ALLOLIB_USE_PORTAUDIO)
    # TODO needs more work on Windows
    if(ALLOLIB_USE_DUMMY_AUDIO)
        message("ALLOLIB_USE_DUMMY_AUDIO ignored. Using PortAudio")
    endif(ALLOLIB_USE_DUMMY_AUDIO)
    target_compile_definitions(al PUBLIC ${PLATFORM_DEFINITION} AL_AUDIO_PORTAUDIO)
    target_link_libraries(al PUBLIC portaudio)
else ()
    if (ALLOLIB_USE_DUMMY_AUDIO)
        target_compile_definitions(al PUBLIC ${PLATFORM_DEFINITION} AL_AUDIO_DUMMY)
    else()
        target_compile_definitions(al PUBLIC ${PLATFORM_DEFINITION} AL_AUDIO_RTAUDIO)

        target_link_libraries(al PUBLIC rtaudio)
    endif(ALLOLIB_USE_DUMMY_AUDIO)
endif()

if(SNDFILE_LIBRARY)
  target_compile_definitions(al PUBLIC AL_LIBSNDFILE)
endif(SNDFILE_LIBRARY)

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL Debug)
    target_compile_options(al PUBLIC
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall>
        $<$<CXX_COMPILER_ID:MSVC>:
        /wd4221 /wd4224 /wd4244  /wd4305  /wd4267 /wd4005 /wd4996>)
    set_target_properties(al PROPERTIES LINK_FLAGS "/ignore:4099")
endif()

target_include_directories(al PUBLIC
  include
  external/Gamma
  external/rtaudio
  external/rtmidi
  external/imgui
  external/glad/include
  external/json/include
  external/glew/include
  external/cpptoml/include
)

target_link_libraries(al PUBLIC
  ${OPENGL_gl_LIBRARY}
  Gamma glfw glad rtmidi cpptoml dr_libs
  nlohmann_json::nlohmann_json imgui oscpack stb serial)

macro(Copy_dlls dest_path target DLLS_TO_COPY)
  if (DLLS_TO_COPY)
    foreach(LIBRARY ${DLLS_TO_COPY})
      get_filename_component(LIBRARY_DIR "${LIBRARY}" DIRECTORY)
      get_filename_component(LIBRARY_NAME "${LIBRARY}" NAME)
      list(APPEND post_build_command robocopy "${LIBRARY_DIR}" "${app_path}" "${LIBRARY_NAME}" &)
    endforeach(LIBRARY IN "${DLLS_TO_COPY}")

    list(APPEND post_build_command IF %ERRORLEVEL% LEQ 1 exit 0)

    add_custom_command(TARGET ${target} POST_BUILD COMMAND ${post_build_command})
  endif ()
endmacro(Copy_dlls)

if (ALLOLIB_BUILD_TESTS)
  message("including allolib test")
  add_subdirectory(test)
endif()

if (ALLOLIB_BUILD_EXAMPLES)
  message("including allolib examples")
  add_subdirectory(examples)
endif()
