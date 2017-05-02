cmake_minimum_required(VERSION 3.0)
project(allo)
set(al_path ${CMAKE_CURRENT_SOURCE_DIR})
include(cmake_modules/configure_platform.cmake)
include(cmake_modules/find_core_dependencies.cmake)

# options
if (WINDOWS)
  # set(CMAKE_CXX_FLAGS "/W1")
else ()
  set(CMAKE_CXX_FLAGS "-std=c++14 -Wall")
endif (WINDOWS)

set(OSCPACK_SRC
  external/oscpack/osc/OscOutboundPacketStream.cpp
  external/oscpack/osc/OscPrintReceivedElements.cpp
  external/oscpack/osc/OscReceivedElements.cpp
  external/oscpack/osc/OscTypes.cpp
)

set(EXTERNAL_INCLUDE_DIR external) # oscpack, etc.
list(APPEND EXTERNAL_SRC
  ${OSCPACK_SRC}
)

set(core_headers
  include/al/core.hpp
  include/al/core/app/al_App.hpp
  include/al/core/app/al_AudioApp.hpp
  include/al/core/app/al_FPS.hpp
  include/al/core/app/al_WindowApp.hpp
  include/al/core/gl/al_BufferObject.hpp
  include/al/core/gl/al_EasyFBO.hpp
  include/al/core/gl/al_EasyVAO.hpp
  include/al/core/gl/al_FBO.hpp
  include/al/core/gl/al_GLEW.hpp
  include/al/core/gl/al_GLFW.hpp
  include/al/core/gl/al_GPUObject.hpp
  include/al/core/gl/al_Graphics.hpp
  include/al/core/gl/al_Lens.hpp
  # include/al/core/gl/al_Light.hpp
  include/al/core/gl/al_Mesh.hpp
  include/al/core/gl/al_Shader.hpp
  include/al/core/gl/al_Shapes.hpp
  # include/al/core/gl/al_Stereographic.hpp
  include/al/core/gl/al_Texture.hpp
  include/al/core/gl/al_VAO.hpp
  include/al/core/gl/al_VAOMesh.hpp
  include/al/core/gl/al_Viewpoint.hpp
  include/al/core/io/al_AudioIO.hpp
  include/al/core/io/al_AudioIOData.hpp
  include/al/core/io/al_ControlNav.hpp
  include/al/core/io/al_Socket.hpp
  include/al/core/io/al_Window.hpp
  include/al/core/math/al_Analysis.hpp
  include/al/core/math/al_Complex.hpp
  include/al/core/math/al_Constants.hpp
  include/al/core/math/al_Frustum.hpp
  include/al/core/math/al_Functions.hpp
  include/al/core/math/al_Interpolation.hpp
  include/al/core/math/al_Interval.hpp
  include/al/core/math/al_Mat.hpp
  include/al/core/math/al_Matrix4.hpp
  include/al/core/math/al_Plane.hpp
  include/al/core/math/al_Quat.hpp
  include/al/core/math/al_Random.hpp
  include/al/core/math/al_Ray.hpp
  include/al/core/math/al_Spherical.hpp
  include/al/core/math/al_Vec.hpp
  include/al/core/protocol/al_OSC.hpp
  include/al/core/protocol/al_Serialize.h
  include/al/core/protocol/al_Serialize.hpp
  include/al/core/spatial/al_Curve.hpp
  include/al/core/spatial/al_DistAtten.hpp
  include/al/core/spatial/al_HashSpace.hpp
  include/al/core/spatial/al_Pose.hpp
  include/al/core/system/al_Config.h
  include/al/core/system/al_Printing.hpp
  include/al/core/system/al_Thread.hpp
  include/al/core/system/al_Time.hpp
  include/al/core/system/al_Watcher.hpp
  include/al/core/types/al_Buffer.hpp
  include/al/core/types/al_Color.hpp
  include/al/core/types/al_Conversion.hpp
  include/al/core/types/al_SingleRWRingBuffer.hpp
)

set(core_sources
  src/core/app/al_AudioApp.cpp
  src/core/app/al_FPS.cpp
  src/core/app/al_WindowApp.cpp
  src/core/gl/al_BufferObject.cpp
  src/core/gl/al_EasyFBO.cpp
  src/core/gl/al_EasyVAO.cpp
  src/core/gl/al_FBO.cpp
  src/core/gl/al_GLEW.cpp
  src/core/gl/al_GLFW.cpp
  src/core/gl/al_GPUObject.cpp
  src/core/gl/al_Graphics.cpp
  src/core/gl/al_Lens.cpp
  # src/core/gl/al_Light.cpp
  src/core/gl/al_Mesh.cpp
  src/core/gl/al_Shader.cpp
  src/core/gl/al_Shapes.cpp
  # src/core/gl/al_Stereographic.cpp
  src/core/gl/al_Texture.cpp
  src/core/gl/al_VAO.cpp
  src/core/gl/al_VAOMesh.cpp
  src/core/gl/al_Viewpoint.cpp
  src/core/io/al_AudioIO.cpp
  src/core/io/al_AudioIOData.cpp
  src/core/io/al_SocketAPR.cpp
  src/core/io/al_ControlNav.cpp
  src/core/io/al_Window.cpp
  src/core/io/al_WindowGLFW.cpp
  src/core/private/al_ImplAPR.h
  src/core/protocol/al_OSC.cpp
  src/core/protocol/al_Serialize.cpp
  src/core/spatial/al_HashSpace.cpp
  src/core/spatial/al_Pose.cpp
  src/core/system/al_Printing.cpp
  src/core/system/al_ThreadNative.cpp
  src/core/system/al_Time.cpp
  src/core/system/al_Watcher.cpp
  src/core/types/al_Color.cpp
)

set(glv_headers
  include/al/glv/glv.h
  include/al/glv/glv_behavior.h
  include/al/glv/glv_buttons.h
  include/al/glv/glv_color.h
  include/al/glv/glv_conf.h
  include/al/glv/glv_core.h
  include/al/glv/glv_draw.h
  include/al/glv/glv_font.h
  # include/al/glv/glv_icon.h
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
  src/glv/glv_buttons.cpp
  src/glv/glv_color.cpp
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

list(APPEND headers
  ${core_headers}
  ${glv_headers}
)

list(APPEND sources
  ${core_sources}
  ${glv_sources}
  ${EXTERNAL_SRC}
)

if (WINDOWS)
  # static libs are considered as archive
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR})
else ()
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif (WINDOWS)

if (WINDOWS)
  add_library(al STATIC ${headers} ${sources})
else ()
  add_library(al STATIC ${headers} ${sources})
endif (WINDOWS)

set_target_properties(al PROPERTIES DEBUG_POSTFIX _debug)

target_include_directories(al PRIVATE include)
target_include_directories(al PRIVATE ${GLFW_INCLUDE_DIRS})
target_include_directories(al PRIVATE ${GLEW_INCLUDE_DIRS})
target_include_directories(al PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
target_include_directories(al PRIVATE ${APR_INCLUDE_DIRS})
target_include_directories(al PRIVATE ${EXTERNAL_INCLUDE_DIR})

target_link_libraries(al ${OPENGL_LIBRARY})
target_link_libraries(al ${GLEW_LIBRARIES})
target_link_libraries(al ${GLFW_LIBRARIES})
target_link_libraries(al ${PORTAUDIO_LIBRARIES})
target_link_libraries(al ${APR_LIBRARIES})

# Testing
add_subdirectory(test)
