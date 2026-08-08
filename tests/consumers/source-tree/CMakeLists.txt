cmake_minimum_required(VERSION 3.24...4.4)
project(allolib_source_tree_consumer LANGUAGES CXX)

set(ALLOLIB_ADD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALLOLIB_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/../../..
  ${CMAKE_CURRENT_BINARY_DIR}/allolib
  EXCLUDE_FROM_ALL
)

add_executable(source_tree_consumer main.cpp)
target_link_libraries(source_tree_consumer PRIVATE al::app)
