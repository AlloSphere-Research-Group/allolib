# sets:
#   EXTERNAL_INCLUDE_DIR
#   EXTERNAL_SRC
#   EXTERNAL_LIBRARIES EXTERNAL_DEBUG_LIBRARIES EXTERNAL_RELEASE_LIBRARIES
#   EXTERNAL_DEFINITIONS


set(AudioAPI "rtaudio" CACHE STRING "Library for Audio IO")
set_property(CACHE AudioAPI PROPERTY STRINGS rtaudio portaudio dummy)

include(${al_path}/cmake/external/oscpack.cmake)
include(${al_path}/cmake/external/gamma.cmake)
include(${al_path}/cmake/external/rtmidi.cmake)

set(EXTERNAL_INCLUDE_DIRS
  ${OSCPACK_INCLUDE_DIR}
  ${GAMMA_INCLUDE_DIR}
  ${RTMIDI_INCLUDE_DIR}
)

set(EXTERNAL_SRC
  ${OSCPACK_SRC}
  ${GAMMA_SRC}
  ${RTMIDI_SRC}
)

set(EXTERNAL_LIBRARIES
  ${OSCPACK_LINK_LIBRARIES}
  ${GAMMA_RELEASE_LIBRARY}
  ${RTMIDI_LIBRARIES}
)

# set(EXTERNAL_DEBUG_LIBRARIES
#   ${GAMMA_DEBUG_LIBRARY}
# )

# set(EXTERNAL_RELEASE_LIBRARIES
#   ${GAMMA_RELEASE_LIBRARY}
# )

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

