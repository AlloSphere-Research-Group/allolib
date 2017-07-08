# sets:
#   EXTERNAL_INCLUDE_DIR
#   EXTERNAL_SRC
#   EXTERNAL_LIBRARIES
#   EXTERNAL_DEFINITIONS

include(${al_path}/cmake_modules/external/oscpack.cmake)
include(${al_path}/cmake_modules/external/rtaudio.cmake)
include(${al_path}/cmake_modules/external/gamma.cmake)

set(EXTERNAL_INCLUDE_DIR
  ${OSCPACK_INCLUDE_DIR}
  ${RTAUDIO_INCLUDE_DIR}
  ${GAMMA_INCLUDE_DIR}
)

set(EXTERNAL_SRC
  ${OSCPACK_SRC}
  ${RTAUDIO_SRC}
)

set(EXTERNAL_LIBRARIES
  ${RTAUDIO_LIBRARIES}
)

set(EXTERNAL_DEFINITIONS
  ${RTAUDIO_DEFINITIONS}
)