cmake_minimum_required(VERSION 3.0)
project(allolib)

set(al_path ${CMAKE_CURRENT_LIST_DIR})
option(AL_VERBOSE_OUTPUT "" OFF)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(AL_MACOS 1)
  set(PLATFORM_DEFINITION -DAL_OSX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(AL_LINUX 1)
  set(PLATFORM_DEFINITION -DAL_LINUX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(AL_WINDOWS 1)
  set(PLATFORM_DEFINITION -DAL_WINDOWS)
else ()
  message(FATAL_ERROR "system platform not defined")
endif ()

include(cmake/find_core_dependencies.cmake)
# sets: CORE_INCLUDE_DIRS, CORE_LIBRARIES, CORE_LIBRARY_DIRS
include(cmake/find_additional_dependencies.cmake)
# sets: ADDITIONAL_INCLUDE_DIRS, ADDITIONAL_LIBRARIES, ADDITIONAL_HEADERS,
#       ADDITIONAL_SOURCES, ADDITIONAL_DEFINITIONS
include(cmake/external.cmake)
# sets: EXTERNAL_INCLUDE_DIRS, EXTERNAL_SRC, EXTERNAL_DEFINITIONS
#       EXTERNAL_LIBRARIES, EXTERNAL_DEBUG_LIBRARIES EXTERNAL_RELEASE_LIBRARIES
include(cmake/al_files.cmake)
# sets: al_headers, al_sources
include(cmake/basic_flags.cmake)
# sets: basic_flags


set(headers
  ${al_headers} ${ADDITIONAL_HEADERS}
)

set(sources
  ${al_sources} ${EXTERNAL_SRC} ${ADDITIONAL_SOURCES}
)

set(dirs_to_include
  ${CORE_INCLUDE_DIRS} ${ADDITIONAL_INCLUDE_DIRS} ${EXTERNAL_INCLUDE_DIRS}
)

set(libs_to_link
  ${CORE_LIBRARIES} ${ADDITIONAL_LIBRARIES} ${EXTERNAL_LIBRARIES}
)

set(definitions
  ${PLATFORM_DEFINITION} ${EXTERNAL_DEFINITIONS} ${ADDITIONAL_DEFINITIONS}
)

set(flags
  ${basic_flags}
)

link_directories(${CORE_LIBRARY_DIRS})

# --- NOW WE ARE READY TO SETUP THE LIBRARY TARGET ---------------------

# multi configuration generators
if (DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
endif ()

add_library(al STATIC ${headers} ${sources})

#paths
set_target_properties(al
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
  ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/Debug
  ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/Release
)

#flags
target_compile_options(al PUBLIC ${flags})

# c++14
set_target_properties(al PROPERTIES CXX_STANDARD 14)
set_target_properties(al PROPERTIES CXX_STANDARD_REQUIRED ON)

# definitions
target_compile_definitions(al PUBLIC ${definitions})

# include dirs
target_include_directories(al PUBLIC
  ${al_path}/include
  ${dirs_to_include}
)

# libs
target_link_libraries(al PUBLIC ${libs_to_link})

# --- Testing -----------------------------------------
add_subdirectory(test)