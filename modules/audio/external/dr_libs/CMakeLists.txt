cmake_minimum_required(VERSION 3.11)
project(dr_libs)

add_library(dr_libs INTERFACE)

target_include_directories(dr_libs INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dr_libs>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/dr_libs>
)
