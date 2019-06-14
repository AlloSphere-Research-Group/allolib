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

set(al_path ${CMAKE_CURRENT_SOURCE_DIR})
option(AL_VERBOSE_OUTPUT "" OFF)
option(BUILD_EXAMPLES "" ON)
option(TRAVIS_BUILD "" OFF)
option(APPVEYOR_BUILD "" OFF)
option(ALLOLIB_BUILD_TESTS "" OFF)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(AL_MACOS 1 CACHE BOOL "Building on OS X")
  set(PLATFORM_DEFINITION -DAL_OSX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(AL_LINUX 1 CACHE BOOL "Building on Linux")
  set(PLATFORM_DEFINITION -DAL_LINUX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(AL_WINDOWS 1 CACHE BOOL "Building on Windows")
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
include(cmake/extensions.cmake)
# sets: EXTENSIONS_LIBRARIES, EXTENSIONS_INCLUDE_DIRS

set(headers
  ${al_headers} ${ADDITIONAL_HEADERS}
)

set(sources
  ${al_sources} ${module_sources} ${EXTERNAL_SRC} ${ADDITIONAL_SOURCES}
)

set(dirs_to_include
  ${ADDITIONAL_INCLUDE_DIRS} ${EXTERNAL_INCLUDE_DIRS} ${EXTENSIONS_INCLUDE_DIRS}
)

set (link_dirs  ${EXTENSIONS_LINK_DIRS})

set(libs_to_link
  ${OPENGL_gl_LIBRARY} ${ADDITIONAL_LIBRARIES} ${EXTERNAL_LIBRARIES} ${EXTENSIONS_LIBRARIES}
)

set(definitions
  ${PLATFORM_DEFINITION} ${EXTERNAL_DEFINITIONS} ${ADDITIONAL_DEFINITIONS} ${EXTENSIONS_DEFINITIONS}
)

# multi configuration generators
if (DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
endif ()

# --- NOW WE ARE READY TO SETUP THE LIBRARY TARGET ---------------------

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

target_include_directories(al PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
)

link_directories(${link_dirs})

target_include_directories(al PUBLIC ${CMAKE_CURRENT_LIST_DIR}/modules)

# use external include dirs when building,
# when installed, external headers will be in `include`
# so only `include` needs to be specified as include directory
foreach (dir ${EXTERNAL_INCLUDE_DIRS})
  target_include_directories(al PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${dir}>"
  )
endforeach (dir)

target_include_directories(al PUBLIC ${dirs_to_include})

# libs
target_link_libraries(al PUBLIC ${libs_to_link} Gamma glfw glew_s)

set_target_properties(al PROPERTIES AL_LINK_LIBRARIES "${libs_to_link}")
set_target_properties(al PROPERTIES AL_DLL_LIBRARIES "${EXTENSIONS_DLLS}")
set_target_properties(al PROPERTIES AL_LINK_DIRECTORIES "${link_dirs}")
set_target_properties(al PROPERTIES AL_INCLUDE_DIRECTORIES "${dirs_to_include}")

# Convenience function
macro(Copy_dlls app_path app_name DLLS_TO_COPY)

  if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message("--- Copying dlls to ${app_path} for target ${app_name}: ${DLLS_TO_COPY}")

    foreach(LIBRARY ${DLLS_TO_COPY})
      get_filename_component(LIBRARY_DIR "${LIBRARY}" DIRECTORY)
      get_filename_component(LIBRARY_NAME "${LIBRARY}" NAME)
      message("${LIBRARY_DIR} ----- ${LIBRARY_NAME}")
      list(APPEND post_build_command
        robocopy "${LIBRARY_DIR}" "${app_path}/bin" "${LIBRARY_NAME}" &
        )
    endforeach(LIBRARY IN "${DLLS_TO_COPY}")

    if (USE_PORTAUDIO)
      list(APPEND post_build_command
        robocopy ${al_path}/dependencies/portaudio/ ${app_path}/bin portaudio_x64.dll &
        )
    endif (USE_PORTAUDIO)

    if (USE_APR)
      list(APPEND post_build_command
        robocopy ${al_path}/dependencies/apr/ ${app_path}/bin libapr-1.dll &
        )
    endif (USE_APR)

    if (FREEIMAGE_FOUND)
      list(APPEND post_build_command
        robocopy ${al_path}/dependencies/FreeImage/Dist/x64 ${app_path}/bin FreeImage.dll &
        )
    endif (FREEIMAGE_FOUND)

    list(APPEND post_build_command
      IF %ERRORLEVEL% LEQ 1 exit 0
      )

    add_custom_command(
      TARGET ${app_name}
      POST_BUILD
      COMMAND ${post_build_command}
      )
  endif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

endmacro(Copy_dlls)


# --- Testing -----------------------------------------
if (ALLOLIB_BUILD_TESTS)
  add_subdirectory(test)
endif()

# ----- Examples -------------------------------------

if (NOT NO_EXAMPLES)
  include("cmake/example_targets.cmake")
endif()

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
        DESTINATION include
)

