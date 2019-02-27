# sets:
#   EXTERNAL_INCLUDE_DIR
#   EXTERNAL_SRC
#   EXTERNAL_LIBRARIES EXTERNAL_DEBUG_LIBRARIES EXTERNAL_RELEASE_LIBRARIES
#   EXTERNAL_DEFINITIONS

# al_path needs to be set prior to calling this script

set(AudioAPI "rtaudio" CACHE STRING "Library for Audio IO")
set_property(CACHE AudioAPI PROPERTY STRINGS rtaudio portaudio dummy)

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

# ---------
include(${al_path}/cmake/external/oscpack.cmake)
include(${al_path}/cmake/external/rtmidi.cmake)
include(${al_path}/cmake/external/imgui.cmake)
include(${al_path}/cmake/external/cpptoml.cmake)

set(EXTERNAL_INCLUDE_DIRS
  ${OSCPACK_INCLUDE_DIR}
  ${RTMIDI_INCLUDE_DIR}
  ${IMGUI_INCLUDE_DIR}
  ${CPPTOML_INCLUDE_DIR}
  ${ext_path}/Gamma
)

set(EXTERNAL_SRC
  ${OSCPACK_SRC}
  ${RTMIDI_SRC}
  ${IMGUI_SRC}
)

set(EXTERNAL_LIBRARIES
  ${OSCPACK_LINK_LIBRARIES}
  ${RTMIDI_LIBRARIES}
)

set(EXTERNAL_DEFINITIONS
  ${RTMIDI_DEFINITIONS}
)

# Real time audio
# Use rtaudio by default

if(AudioAPI STREQUAL "rtaudio")
  include(${al_path}/cmake/external/rtaudio.cmake)

  list(APPEND EXTERNAL_INCLUDE_DIRS
    ${RTAUDIO_INCLUDE_DIR}
    )

  list(APPEND EXTERNAL_LIBRARIES
    ${RTAUDIO_LIBRARIES}
    )

  list(APPEND EXTERNAL_SRC
    ${RTAUDIO_SRC}
    )

  list(APPEND EXTERNAL_DEFINITIONS
    ${RTAUDIO_DEFINITIONS}
    -DAL_AUDIO_RTAUDIO
    )

elseif(AudioAPI STREQUAL "portaudio")
  if (AL_WINDOWS)
      set(PORTAUDIO_INCLUDE_DIRS ${al_path}/dependencies/portaudio/include)
      set(PORTAUDIO_LIBRARIES ${al_path}/dependencies/portaudio/portaudio_x64.lib)
  else ()
      pkg_search_module(PORTAUDIO REQUIRED portaudio-2.0)
  endif (AL_WINDOWS)

  list(APPEND EXTERNAL_LIBRARIES
    ${PORTAUDIO_LIBRARIES}
    )

  list(APPEND EXTERNAL_DEFINITIONS
    -DAL_AUDIO_PORTAUDIO
    ${PORTAUDIO_DEFINITIONS}
    )

  list(APPEND EXTERNAL_INCLUDE_DIR
    ${PORTAUDIO_INCLUDE_DIRS}
    )

else(AudioAPI STREQUAL "dummy") # Dummy audio backend
  list(APPEND EXTERNAL_DEFINITIONS
    -DAL_AUDIO_DUMMY
    )
endif()

