cmake_minimum_required(VERSION 3.0)
project(allo)

# select modules to include in building
set(build_audio 0)
set(build_gl 0)
set(build_sphere 0)
set(build_util 0)

set(gl_profile "core") # or "compat"
set(build_cv 1)

set(CMAKE_BUILD_TYPE Debug)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")

include_directories(include)

# build core
# should not have dependencies, except c++11 (14?)

set(core_headers
	include/al/core/math/al_Constants.hpp
	include/al/core/math/al_Functions.hpp
	include/al/core/math/al_Mat.hpp
	include/al/core/math/al_Matrix4.hpp
	include/al/core/math/al_Quat.hpp
	include/al/core/math/al_Vec.hpp
	include/al/core/types/al_Color.hpp
)

set(core_sources
	src/core/types/al_Color.cpp
)

# build audio
# dependencies: allocore, portaudio, ?, ?
if(build_audio)
	find_libs
	gamma
endif(build_audio)

# build graphics
# dependencies: allocore, opengl
# different files depending on gl version
if(build_gl)
	lfind opengl


endif(build_gl)

# build sphere
# dependencies: allocore, omnirender, cuttlebone
if(build_sphere)
	cuttlbone
endif(build_sphere)

# build util
#dependencies: allocore, ?, ?
if(build_util)
	glv
endif(build_util)

list(APPEND headers ${core_headers})
list(APPEND sources ${core_sources})

# shared? static?
add_library(allo ${headers} ${sources})
