cmake_minimum_required(VERSION 3.0)
project(allo)

# CHECK PLATFORM ###########################################
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(MACOS 1)
  message("MACOS")
  add_definitions(-DAL_OSX)
elseif (WIN32)
  set(WINDOWS 1)
  message("WINDOWS")
  add_definitions(-DAL_WINDOWS)
elseif (CYGWIN)
  set(WINDOWS_CYGWIN 1)
  message("WINDWOS_CYGWIN")
  # later change to AL_WINDOWS_CYGWIN?
  add_definitions(-DAL_WINDOWS)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX 1)
  message("LINUX")
  add_definitions(-DAL_LINUX)
else ()
endif ()

# original allosystem version
# if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#   add_definitions(-DAL_OSX)
# elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#   add_definitions(-DAL_LINUX)
# elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
#   add_definitions(-DAL_WINDOWS)
# endif()

set(CMAKE_BUILD_TYPE Debug)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wno-unused")

# build core
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

set(core_headers
  include/al/core/gl/al_GPUObject.hpp
  include/al/core/gl/al_Graphics.hpp
  include/al/core/gl/al_Lens.hpp
  include/al/core/gl/al_Mesh.hpp
  include/al/core/gl/al_OpenGL.hpp
  include/al/core/gl/al_Shader.hpp
  include/al/core/gl/al_Shapes.hpp
  include/al/core/gl/al_Texture.hpp
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
  include/al/core/math/al_Ray.hpp
  include/al/core/math/al_Spherical.hpp
  include/al/core/math/al_Vec.hpp
  include/al/core/spatial/al_Curve.hpp
  include/al/core/spatial/al_DistAtten.hpp
  include/al/core/spatial/al_HashSpace.hpp
  include/al/core/spatial/al_Pose.hpp
  include/al/core/system/al_Config.h
  include/al/core/system/al_Printing.hpp
  include/al/core/system/pstdint.h
  include/al/core/types/al_Array.h
  include/al/core/types/al_Array.hpp
  include/al/core/types/al_Buffer.hpp
  include/al/core/types/al_Color.hpp
  include/al/core/types/al_Conversion.hpp
  include/al/core/types/al_SingleRWRingBuffer.hpp
)

set(core_sources
  src/core/gl/al_GPUObject.cpp
  src/core/gl/al_Graphics.cpp
  src/core/gl/al_Lens.cpp
  src/core/gl/al_Mesh.cpp
  src/core/gl/al_Shader.cpp
  src/core/gl/al_Shapes.cpp
  src/core/gl/al_Texture.cpp
  src/core/spatial/al_HashSpace.cpp
  src/core/spatial/al_Pose.cpp
  src/core/system/al_Printing.cpp
  src/core/types/al_Array.cpp
  src/core/types/al_Array_C.c
  src/core/types/al_Color.cpp
)

# audio

# graphics
set(graphics_headers
)
set(graphics_sources
)

# AlloSphere

# utils

list(APPEND headers
  ${core_headers}
  ${graphics_headers}
)
list(APPEND sources
  ${core_sources}
  ${graphics_sources}
)

include_directories(include)
include_directories(${GLEW_INCLUDE_DIRS})

# shared? static?
add_library(al ${headers} ${sources})

target_link_libraries(al ${OPENGL_LIBRARY})
target_link_libraries(al ${GLEW_LIBRARIES})