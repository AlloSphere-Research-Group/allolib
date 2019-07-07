cmake_minimum_required(VERSION 3.0)
project(dr_libs)

add_library(dr_libs INTERFACE)

target_include_directories(dr_libs INTERFACE dr_libs)