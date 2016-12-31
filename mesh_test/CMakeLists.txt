cmake_minimum_required(VERSION 3.0)
project(proj_al_test)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wno-unused")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

add_executable(al_test main.cpp)

target_include_directories(al_test PRIVATE include)
target_include_directories(al_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(al_test PRIVATE ${GLFW_INCLUDE_DIRS})
target_include_directories(al_test PRIVATE ${GLEW_INCLUDE_DIRS})

target_link_libraries(al_test ${CMAKE_CURRENT_SOURCE_DIR}/../libal.a)
target_link_libraries(al_test ${OPENGL_LIBRARY})
target_link_libraries(al_test ${GLEW_LIBRARIES})
target_link_libraries(al_test ${GLFW_LIBRARIES})
