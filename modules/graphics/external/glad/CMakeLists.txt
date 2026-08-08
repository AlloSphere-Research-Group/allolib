cmake_minimum_required(VERSION 3.11)
project(glad)

if(ALLOLIB_BUILD_SHARED)
  set(ALLOLIB_EXTERNAL_LIB_TYPE "SHARED")
else()
  set(ALLOLIB_EXTERNAL_LIB_TYPE "STATIC")
endif()

add_library(glad ${ALLOLIB_EXTERNAL_LIB_TYPE} src/glad.c)

target_include_directories(glad PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(AL_WINDOWS)
  target_compile_definitions(glad PUBLIC NOMINMAX)
endif()

set_target_properties(glad PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

if(UNIX)
  target_link_libraries(glad PUBLIC dl)
endif()
