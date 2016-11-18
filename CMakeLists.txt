cmake_minimum_required(VERSION 3.0)
project(allo)

# CHECK PLATFORM ###########################################
if (APPLE)
  set(MACOS 1)
  message("MACOS")
  add_definitions(-DAL_OSX)
elseif (UNIX)
  set(LINUX 1)
  message("LINUX")
  add_definitions(-DAL_LINUX)
else ()
  set(WINDOWS 1)
  message("WINDOWS")
  add_definitions(-DAL_WINDOWS)
endif (APPLE)

# original allosystem version
# if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#   add_definitions(-DAL_OSX)
# elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#   add_definitions(-DAL_LINUX)
# elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
#   add_definitions(-DAL_WINDOWS)
# endif()

# select gl loader
set(gl_loader "glew") # or "raw" or "gl3w" ???

# select modules to include in building
set(build_audio 0)
set(build_gl 0)
set(build_sphere 0)
set(build_util 0)

set(gl_profile "core") # or "compat" ???
# set(build_cv 1)

set(CMAKE_BUILD_TYPE Debug)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wno-unused")

include_directories(include) # so users can say #include al/core.hpp

# build core
# should not have dependencies, except c++11 (c++14 will be cool...)

set(core_headers
  include/al/core/graphics/al_GPUObject.hpp
  include/al/core/graphics/al_Graphics.hpp
  include/al/core/graphics/al_Lens.hpp
  include/al/core/graphics/al_Mesh.hpp
  include/al/core/graphics/al_Shapes.hpp
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
  src/core/graphics/al_GPUObject.cpp
  src/core/graphics/al_Graphics.cpp
  src/core/graphics/al_Lens.cpp
  src/core/graphics/al_Mesh.cpp
  src/core/graphics/al_Shapes.cpp
  src/core/spatial/al_HashSpace.cpp
  src/core/spatial/al_Pose.cpp
  src/core/system/al_Printing.cpp
  src/core/types/al_Array.cpp
  src/core/types/al_Array_C.c
  src/core/types/al_Color.cpp
)

# build audio
# dependencies: allocore, portaudio, ?, ?
if(build_audio)
  # find_libs
  # gamma
endif(build_audio)

# build graphics
# dependencies: allocore, opengl
# different files depending on gl version
if(build_gl)
  # lfind opengl
endif(build_gl)

# build sphere
# dependencies: allocore, omnirender, cuttlebone
if(build_sphere)
  # cuttlbone, 
endif(build_sphere)

# build util
#dependencies: allocore, ?, ?
if(build_util)
  # glv
endif(build_util)

list(APPEND headers ${core_headers})
list(APPEND sources ${core_sources})

# shared? static?
add_library(al ${headers} ${sources})
