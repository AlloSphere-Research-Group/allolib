cmake_minimum_required(VERSION 3.0)
project(glad)

add_library(glad STATIC src/glad.c)

target_include_directories(glad PUBLIC include)

set_target_properties(glad PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

if (UNIX)
    target_link_libraries(glad PUBLIC dl)
endif (UNIX)