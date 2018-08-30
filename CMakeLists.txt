cmake_minimum_required(VERSION 3.0)
project(allolib)

MACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD MSG)
     STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}"
     "${CMAKE_BINARY_DIR}" insource)
     GET_FILENAME_COMPONENT(PARENTDIR ${CMAKE_SOURCE_DIR} PATH)
     STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}"
     "${PARENTDIR}" insourcesubdir)
    IF(insource OR insourcesubdir)
        MESSAGE(FATAL_ERROR "${MSG}")
    ENDIF(insource OR insourcesubdir)
ENDMACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD)

MACRO_ENSURE_OUT_OF_SOURCE_BUILD(
    "${CMAKE_PROJECT_NAME} requires an out of source build. Make a directory and run cmake there."
)

set(al_path ${CMAKE_CURRENT_LIST_DIR})
option(AL_VERBOSE_OUTPUT "" OFF)
option(BUILD_EXAMPLES "" ON)
option(TRAVIS_BUILD "" OFF)
option(APPVEYOR_BUILD "" OFF)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(AL_MACOS 1)
  set(PLATFORM_DEFINITION -DAL_OSX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(AL_LINUX 1)
  set(PLATFORM_DEFINITION -DAL_LINUX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(AL_WINDOWS 1)
  set(PLATFORM_DEFINITION -DAL_WINDOWS -D_USE_MATH_DEFINES)
  if (NOT ${CMAKE_GENERATOR_PLATFORM} STREQUAL "x64")
  message("Warning! Not building for Win64 : ${CMAKE_GENERATOR_PLATFORM}")
  endif(NOT ${CMAKE_GENERATOR_PLATFORM} STREQUAL "x64")
else ()
  message(FATAL_ERROR "system platform not defined")
endif ()

if(TRAVIS_BUILD) 
  add_definitions(-DTRAVIS_BUILD)
endif(TRAVIS_BUILD)

# include(cmake/find_core_dependencies.cmake)
# sets: CORE_INCLUDE_DIRS, CORE_LIBRARIES, CORE_LIBRARY_DIRS
include(cmake/find_additional_dependencies.cmake)
# sets: ADDITIONAL_INCLUDE_DIRS, ADDITIONAL_LIBRARIES, ADDITIONAL_HEADERS,
#       ADDITIONAL_SOURCES, ADDITIONAL_DEFINITIONS
include(cmake/external.cmake)
# sets: EXTERNAL_INCLUDE_DIRS, EXTERNAL_SRC, EXTERNAL_DEFINITIONS
#       EXTERNAL_LIBRARIES, EXTERNAL_DEBUG_LIBRARIES EXTERNAL_RELEASE_LIBRARIES
include(cmake/al_files.cmake)
# sets: al_headers, al_sources

set(headers
  ${al_headers} ${ADDITIONAL_HEADERS}
)

set(sources
  ${al_sources} ${module_sources} ${EXTERNAL_SRC} ${ADDITIONAL_SOURCES}
)

set(dirs_to_include
  ${ADDITIONAL_INCLUDE_DIRS} ${EXTERNAL_INCLUDE_DIRS}
)

set(libs_to_link
  ${OPENGL_gl_LIBRARY} ${ADDITIONAL_LIBRARIES} ${EXTERNAL_LIBRARIES}
)

set(definitions
  ${PLATFORM_DEFINITION} ${EXTERNAL_DEFINITIONS} ${ADDITIONAL_DEFINITIONS}
)

# multi configuration generators
if (DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
endif ()

# --- NOW WE ARE READY TO SETUP THE LIBRARY TARGET ---------------------

# first build dependencies

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(BUILD_UTILS OFF CACHE BOOL "" FORCE) # for glew

set(ext_path ${al_path}/external)

if (DEFINED CMAKE_CONFIGURATION_TYPES)
  add_subdirectory(${ext_path}/Gamma ${CMAKE_CURRENT_BINARY_DIR}/Gamma)
  add_subdirectory(${ext_path}/glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw)
  add_subdirectory(${ext_path}/glew/build/cmake ${CMAKE_CURRENT_BINARY_DIR}/glew)
else()
  if (${CMAKE_BUILD_TYPE} MATCHES Debug)
    add_subdirectory(${ext_path}/Gamma ${CMAKE_CURRENT_BINARY_DIR}/Gamma)
    add_subdirectory(${ext_path}/glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw)
    add_subdirectory(${ext_path}/glew/build/cmake ${CMAKE_CURRENT_BINARY_DIR}/glew)
  else()
    add_subdirectory(${ext_path}/Gamma ${CMAKE_CURRENT_BINARY_DIR}/Gamma)
    add_subdirectory(${ext_path}/glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw)
    add_subdirectory(${ext_path}/glew/build/cmake ${CMAKE_CURRENT_BINARY_DIR}/glew)
  endif()
endif ()

# then allolib library
add_library(al STATIC ${headers} ${sources})

#paths
set_target_properties(al
  PROPERTIES
  DEBUG_POSTFIX d
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib
  ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/lib
  ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/lib
)

#flags
if (AL_WINDOWS)
  target_compile_options(al PRIVATE "")
else ()
  target_compile_options(al PRIVATE "-Wall")
endif (AL_WINDOWS)

# c++14
set_target_properties(al PROPERTIES CXX_STANDARD 14)
set_target_properties(al PROPERTIES CXX_STANDARD_REQUIRED ON)

# definitions
target_compile_definitions(al PUBLIC ${definitions})

# include dirs
# target_include_directories(al PUBLIC
#   ${al_path}/include
#   ${dirs_to_include}
# )

target_include_directories(al PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
)

# use external include dirs when building,
# when installed, external headers will be in `include`
# so only `include` needs to be specified as include directory
foreach (dir ${EXTERNAL_INCLUDE_DIRS})
  target_include_directories(al PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${dir}>"
  )
endforeach (dir)

target_include_directories(al PUBLIC ${ADDITIONAL_INCLUDE_DIRS})

# libs
target_link_libraries(al PUBLIC ${libs_to_link} Gamma glfw glew_s)

# --- Testing -----------------------------------------
if (ALLOLIB_BUILD_TESTS)
  add_subdirectory(test)
endif()

# ----- Examples -------------------------------------

include("cmake/example_targets.cmake")

# ----- INSTALL ----------------------------------------------------------------
#
# [!] dependent targets (glew_s, glfw, and Gamma) shoud have install target also

set(CMAKE_INSTALL_PREFIX ${al_path}/al)

install(TARGETS al
  EXPORT al-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# For some reason this line causes an error when including depencies that are inside the source
# tree on windows.... What does it do anyway...?
#install(EXPORT al-targets DESTINATION lib/cmake/al)
install(DIRECTORY ${al_path}/include/al DESTINATION include)

install(DIRECTORY external/oscpack/ip DESTINATION include)
install(DIRECTORY external/oscpack/osc DESTINATION include)
install(FILES external/imgui/imgui.h DESTINATION include)
install(FILES external/imgui/imconfig.h DESTINATION include)
install(FILES external/rtaudio/RtAudio.h DESTINATION include)
install(FILES external/rtmidi/rtmidi_c.h external/rtmidi/RtMidi.h
        DESTINATION include)
