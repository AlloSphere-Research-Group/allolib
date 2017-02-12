cmake_minimum_required(VERSION 3.0)
project(allo)

# CHECK PLATFORM ###########################################
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(MACOS 1)
  message("MACOS")
  add_definitions(-DAL_OSX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX 1)
  message("LINUX")
  add_definitions(-DAL_LINUX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  if (MSYS)
    set(WINDOWS_CYGWIN 1)
    message("WINDOWS_CYGWIN")
    # later change to AL_WINDOWS_MSYS?
    add_definitions(-DAL_WINDOWS)
  else () # WIN32
    set(WINDOWS 1)
    message("WINDOWS")
    add_definitions(-DAL_WINDOWS)
  endif()
else ()
endif ()

# options
# AL_ENABLE_DEBUG

# set(CMAKE_BUILD_TYPE Debug)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
if (WINDOWS)
  set(CMAKE_CXX_FLAGS "/W1")
else ()
  set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wno-unused")
endif (WINDOWS)

# core dependencies
find_package(OpenGL REQUIRED)

if (WINDOWS)
  set(GLFW_INCLUDE_DIRS libs/glfw/include)
  set(GLFW_LIBRARIES libs/glfw/glfw3dll.lib)
  set(GLEW_INCLUDE_DIRS libs/glew/include)
  set(GLEW_LIBRARIES libs/glew/glew32.lib)
  set(PORTAUDIO_INCLUDE_DIRS libs/portaudio/include)
  set(PORTAUDIO_LIBRARIES libs/portaudio/portaudio_x64.lib)
  set(APR_INCLUDE_DIR libs/apr/include)
  set(APR_LIBRARY libs/apr/libapr-1.lib)
else ()
  find_package(GLEW REQUIRED)
  find_package(PkgConfig REQUIRED)
  pkg_search_module(GLFW REQUIRED glfw3)
  pkg_search_module(PORTAUDIO REQUIRED portaudio-2.0)
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
  include/al/core/app/al_AudioApp.hpp
  include/al/core/app/al_AudioIO.hpp
  include/al/core/app/al_AudioIOData.hpp
  include/al/core/app/al_FPS.hpp
  include/al/core/app/al_Window.hpp
  include/al/core/app/al_WindowApp.hpp
  include/al/core/gl/al_BufferObject.hpp
  # include/al/core/gl/al_CubeMapFBO.hpp
  include/al/core/gl/al_FBO.hpp
  include/al/core/gl/al_GLEW.hpp
  include/al/core/gl/al_GLFW.hpp
  include/al/core/gl/al_GPUObject.hpp
  include/al/core/gl/al_Graphics.hpp
  include/al/core/gl/al_Lens.hpp
  include/al/core/gl/al_Light.hpp
  include/al/core/gl/al_Mesh.hpp
  include/al/core/gl/al_Shader.hpp
  # include/al/core/gl/al_Shapes.hpp
  include/al/core/gl/al_Stereographic.hpp
  include/al/core/gl/al_Texture.hpp
  include/al/core/gl/al_VAO.hpp
  include/al/core/gl/al_VAOMesh.hpp
  include/al/core/io/al_Socket.hpp
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
  src/core/app/al_AudioIO.cpp
  src/core/app/al_AudioIOData.cpp
  src/core/app/al_FPS.cpp
  src/core/app/al_Window.cpp
  src/core/app/al_WindowApp.cpp
  src/core/app/al_WindowGLFW.cpp
  src/core/gl/al_BufferObject.cpp
  src/core/gl/al_FBO.cpp
  src/core/gl/al_GLEW.cpp
  src/core/gl/al_GLFW.cpp
  src/core/gl/al_GPUObject.cpp
  src/core/gl/al_Graphics.cpp
  src/core/gl/al_Lens.cpp
  src/core/gl/al_Light.cpp
  src/core/gl/al_Mesh.cpp
  src/core/gl/al_Shader.cpp
  # src/core/gl/al_Shapes.cpp
  src/core/gl/al_Stereographic.cpp
  src/core/gl/al_Texture.cpp
  src/core/gl/al_VAO.cpp
  src/core/gl/al_VAOMesh.cpp
  src/core/io/al_SocketAPR.cpp
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

# audio

# graphics
set(graphics_headers
)
set(graphics_sources
)

# AlloSphere

# ui

# util

list(APPEND headers
  ${core_headers}
  # ${graphics_headers}
)

list(APPEND sources
  ${core_sources}
  ${EXTERNAL_SRC}
  # ${graphics_sources}
)

if (WINDOWS)
  add_library(al STATIC ${headers} ${sources})
else ()
  add_library(al STATIC ${headers} ${sources})
endif (WINDOWS)


target_include_directories(al PRIVATE include)
target_include_directories(al PRIVATE ${GLFW_INCLUDE_DIRS})
target_include_directories(al PRIVATE ${GLEW_INCLUDE_DIRS})
target_include_directories(al PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
target_include_directories(al PRIVATE ${APR_INCLUDE_DIR})
target_include_directories(al PRIVATE ${EXTERNAL_INCLUDE_DIR})

target_link_libraries(al ${OPENGL_LIBRARY})
target_link_libraries(al ${GLEW_LIBRARIES})
target_link_libraries(al ${GLFW_LIBRARIES})
target_link_libraries(al ${PORTAUDIO_LIBRARIES})
target_link_libraries(al ${APR_LIBRARY})