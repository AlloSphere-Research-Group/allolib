# sets:
#   ADDITIONAL_INCLUDE_DIRS
#   ADDITIONAL_LIBRARIES
# 	ADDITIONAL_HEADERS
# 	ADDITIONAL_SOURCES
#	  ADDITIONAL_DEFINITIONS

option(USE_APR "" OFF)

# al_path needs to be set prior to calling this script

if (AL_WINDOWS)
  if (USE_APR)
    set(APR_INCLUDE_DIRS ${al_path}/dependencies/apr/include)
    set(APR_LIBRARIES ${al_path}/dependencies/apr/libapr-1.lib)
  endif (USE_APR)

else ()

  if (USE_APR)
    pkg_search_module(APR REQUIRED apr-1)
  endif (USE_APR)
  
endif (AL_WINDOWS)

if (USE_APR)
  set(APR_HEADERS
    ${al_path}/include/al/util/al_Socket.hpp
  )
  set(APR_SOURCES
    src/util/al_SocketAPR.cpp
  )
endif(USE_APR)

set(ADDITIONAL_INCLUDE_DIRS
	${PORTAUDIO_INCLUDE_DIRS}
	${APR_INCLUDE_DIRS}
)

set(ADDITIONAL_LIBRARIES
	${PORTAUDIO_LIBRARIES}
	${APR_LIBRARIES}
)

set(ADDITIONAL_HEADERS
	${APR_HEADERS}
)

set(ADDITIONAL_SOURCES
	${APR_SOURCES}
)

set(ADDITIONAL_DEFINITIONS
	${PORTAUDIO_DEFINITIONS}
)
