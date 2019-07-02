cmake_minimum_required(VERSION 3.0)

set(APP_NAME empty_app)

project(${APP_NAME})

#set(APP_OPTION OFF CACHE BOOL "description" FORCE)

add_subdirectory(.. al)

add_executable(${APP_NAME} main.cpp)

set_target_properties(${APP_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH .
    DEBUG_POSTFIX d
)

#target_include_directories(${APP_NAME} PRIVATE external/lib/include)
target_link_libraries(${APP_NAME} PRIVATE al)

