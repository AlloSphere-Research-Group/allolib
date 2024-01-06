cmake_minimum_required(VERSION 3.5)
project(stb)

add_library(stb INTERFACE)

target_include_directories(stb INTERFACE stb)