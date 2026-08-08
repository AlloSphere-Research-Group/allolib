cmake_minimum_required(VERSION 3.24...4.4)
project(allolib_install_tree_consumer LANGUAGES CXX)

find_package(AlloLib CONFIG REQUIRED)

add_executable(install_tree_consumer main.cpp)
target_link_libraries(install_tree_consumer PRIVATE al::app)

# RtMidi/RtAudio may still surface jack on some platforms.
if(APPLE AND EXISTS "/opt/homebrew/lib")
  target_link_directories(install_tree_consumer PRIVATE "/opt/homebrew/lib")
endif()
