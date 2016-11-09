cmake_minimum_required(VERSION 3.0)
project(proj_al_test)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_executable(al_test main.cpp)

target_link_libraries(al_test ${CMAKE_CURRENT_SOURCE_DIR}/../liballo.a)