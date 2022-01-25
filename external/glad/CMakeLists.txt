cmake_minimum_required(VERSION 3.0)
project(glad)

if(ALLOLIB_BUILD_SHARED)
    set(ALLOLIB_EXTERNAL_LIB_TYPE "SHARED")
else()
    set(ALLOLIB_EXTERNAL_LIB_TYPE "STATIC")
endif()

add_library(glad ${ALLOLIB_EXTERNAL_LIB_TYPE} src/glad.c)

target_include_directories(glad PUBLIC include)

set_target_properties(glad PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

if (UNIX)
    target_link_libraries(glad PUBLIC dl)
endif (UNIX)
