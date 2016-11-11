cmake_minimum_required(VERSION 3.0)
project(proj_al_test)

# CHECK PLATFORM ###########################################
if (APPLE)
	set(MACOS 1)
	message("MACOS")
elseif (UNIX)
	set(LINUX 1)
	message("LINUX")
else ()
	set(WINDOWS 1)
	message("WINDOWS")
endif (APPLE)

# SETTINGS #################################################
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wno-unused")

# FIND PACKAGES ############################################
find_package(OpenGL REQUIRED)
if (WINDOWS)
else ()
	find_package(GLEW REQUIRED)
	find_package(PkgConfig REQUIRED)
	pkg_search_module(GLFW REQUIRED glfw3)
	if (MACOS)
	endif (MACOS)
endif (WINDOWS)

# INCLUDE DIRECTORIES ######################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})

add_executable(al_gl_test main.cpp)

target_link_libraries(al_gl_test ${CMAKE_CURRENT_SOURCE_DIR}/../libal.a)
target_link_libraries(al_gl_test ${OPENGL_LIBRARY})
if (MACOS)
endif (MACOS)
if (LINUX)
endif (LINUX)
target_link_libraries(al_gl_test ${GLFW_LIBRARIES})
target_link_libraries(al_gl_test ${GLEW_LIBRARIES})
