cmake_minimum_required(VERSION 3.6)

project(allolib)
set(al_path ${CMAKE_CURRENT_SOURCE_DIR})
# set(BUILDING_ALLOLIB 1)

include(cmake_modules/configure_platform.cmake)
# sets: AL_MACOS || AL_LINUX || AL_WINDOWS, and PLATFORM_DEFINITION
include(cmake_modules/find_core_dependencies.cmake)
# sets: CORE_INCLUDE_DIRS, CORE_LIBRARIES, CORE_LIBRARY_DIRS
include(cmake_modules/find_additional_dependencies.cmake)
# sets: ADDITIONAL_INCLUDE_DIRS, ADDITIONAL_LIBRARIES, ADDITIONAL_HEADERS,
#       ADDITIONAL_SOURCES, ADDITIONAL_DEFINITIONS
include(cmake_modules/external.cmake)
# sets: EXTERNAL_INCLUDE_DIRS, EXTERNAL_SRC, EXTERNAL_DEFINITIONS
#       EXTERNAL_LIBRARIES, EXTERNAL_DEBUG_LIBRARIES EXTERNAL_RELEASE_LIBRARIES
include(cmake_modules/al_files.cmake)
# sets: al_headers, al_sources
include(cmake_modules/basic_flags.cmake)
# sets: basic_flags


set(headers
  ${al_headers} ${ADDITIONAL_HEADERS}
)

set(sources
  ${al_sources} ${EXTERNAL_SRC} ${ADDITIONAL_SOURCES}
)

# set(dirs_to_include
#   ${CORE_INCLUDE_DIRS} ${ADDITIONAL_INCLUDE_DIRS} ${EXTERNAL_INCLUDE_DIRS}
# )

set(dirs_to_include
  ${ADDITIONAL_INCLUDE_DIRS} ${EXTERNAL_INCLUDE_DIRS}
)

# set(libs_to_link
#   ${CORE_LIBRARIES} ${ADDITIONAL_LIBRARIES} ${EXTERNAL_LIBRARIES}
# )

set(libs_to_link
  ${ADDITIONAL_LIBRARIES} ${EXTERNAL_LIBRARIES}
)

# set(debug_libs_to_link
#   ${EXTERNAL_DEBUG_LIBRARIES}
# )

# set(release_libs_to_link
#   ${EXTERNAL_RELEASE_LIBRARIES}
# )

set(definitions
  ${PLATFORM_DEFINITION} ${EXTERNAL_DEFINITIONS} ${ADDITIONAL_DEFINITIONS}
)

set(flags
  ${basic_flags}
)

# link_directories(${CORE_LIBRARY_DIRS})

# --- NOW WE ARE READY TO SETUP THE LIBRARY TARGET ---------------------

# multi configuration generators
if (MSVC)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
endif ()

# add_library(al STATIC ${headers} ${sources})
add_library(al STATIC "")
# add_library(al STATIC ${sources})
# add_library(al STATIC ${headers})
target_sources(al PRIVATE ${sources})
target_sources(al PRIVATE ${headers})

#paths
set_target_properties(al
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

#flags
target_compile_options(al PUBLIC ${flags}) # public because of headers

# c++14
set_target_properties(al PROPERTIES CXX_STANDARD 14)
set_target_properties(al PROPERTIES CXX_STANDARD_REQUIRED ON)

# definitions
target_compile_definitions(al PRIVATE ${definitions})

# naming
set_target_properties(al PROPERTIES DEBUG_POSTFIX _debug)

# include dirs
target_include_directories(al PRIVATE include)
target_include_directories(al PRIVATE ${dirs_to_include})

# libs
target_link_libraries(al ${libs_to_link})
target_link_libraries(al
    GLEW::GLEW
    PkgConfig::GLFW
    ${OPENGL_LIBRARY}
)
# target_link_libraries(
#   al
#   debug ${debug_libs_to_link}
#   optimized ${release_libs_to_link}
# )

# --- Testing -----------------------------------------
add_subdirectory(test)