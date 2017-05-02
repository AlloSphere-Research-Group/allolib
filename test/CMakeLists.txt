set (test_src
    src/main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/external/catch)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(al_tests ${test_src})
target_link_libraries(al_tests al)
