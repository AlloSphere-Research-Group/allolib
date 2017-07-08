cmake_minimum_required(VERSION 3.0)
project(allo)
set(al_path ${CMAKE_CURRENT_SOURCE_DIR})

include(cmake_modules/configure_platform.cmake)
# sets: MACOS || LINUX || WINDOWS_MINGW || WINDOWS, and PLATFORM_DEFINITION
include(cmake_modules/find_core_dependencies.cmake)
# sets: CORE_INCLUDE_DIRS, CORE_LIBRARIES
include(cmake_modules/find_additional_dependencies.cmake)
# sets: ADDITIONAL_INCLUDE_DIRS, ADDITIONAL_LIBRARIES, ADDITIONAL_HEADERS,
#       ADDITIONAL_SOURCES, ADDITIONAL_DEFINITIONS
include(cmake_modules/external.cmake)
# sets: EXTERNAL_INCLUDE_DIR, EXTERNAL_SRC, EXTERNAL_LIBRARIES, EXTERNAL_DEFINITIONS
include(cmake_modules/al_files.cmake)
# sets: al_headers, al_sources

set(headers
  ${al_headers}
  ${ADDITIONAL_HEADERS}
)

set(sources
  ${al_sources}
  ${EXTERNAL_SRC}
  ${ADDITIONAL_SOURCES}
)

set(dirs_to_include
  ${CORE_INCLUDE_DIRS}
  ${ADDITIONAL_INCLUDE_DIRS}
  ${EXTERNAL_INCLUDE_DIR}
)

set(libs_to_link
  ${CORE_LIBRARIES}
  ${ADDITIONAL_LIBRARIES}
  ${EXTERNAL_LIBRARIES}
)

set(definitions
  ${PLATFORM_DEFINITION}
  ${EXTERNAL_DEFINITIONS}
  ${ADDITIONAL_DEFINITIONS}
)

# --- NOW WE ARE READY TO SETUP THE LIBRARY TARGET ----------

# definitions
add_definitions(
  ${definitions}
)

# flags
if (WINDOWS)
  # set(CMAKE_CXX_FLAGS "/W1")
else ()
  set(CMAKE_CXX_FLAGS "-std=c++14 -Wall")
endif (WINDOWS)

# paths
if (WINDOWS)
  # in windows, static libs are called archive
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR})
else ()
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif (WINDOWS)

# target
if (WINDOWS)
  add_library(al STATIC ${headers} ${sources})
else ()
  add_library(al STATIC ${headers} ${sources})
endif (WINDOWS)

# naming
set_target_properties(al PROPERTIES DEBUG_POSTFIX _debug)

# include dirs
target_include_directories(al PRIVATE include)
target_include_directories(al PRIVATE ${dirs_to_include})

# libs
target_link_libraries(al ${libs_to_link})

# Testing
add_subdirectory(test)