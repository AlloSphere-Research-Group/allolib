# sets:
#   ADDITIONAL_INCLUDE_DIRS
#   ADDITIONAL_LIBRARIES
# 	ADDITIONAL_HEADERS
# 	ADDITIONAL_SOURCES
#	ADDITIONAL_DEFINITIONS

option(USE_PORTAUDIO "" OFF)
option(USE_APR "" OFF)

# al_path needs to be set prior to calling this script

if (WINDOWS)

  if (USE_PORTAUDIO)
    set(PORTAUDIO_INCLUDE_DIRS ${al_path}/dependencies/portaudio/include)
    set(PORTAUDIO_LIBRARIES ${al_path}/dependencies/portaudio/portaudio_x64.lib)
  endif (USE_PORTAUDIO)

  if (USE_APR)
    set(APR_INCLUDE_DIRS ${al_path}/dependencies/apr/include)
    set(APR_LIBRARIES ${al_path}/dependencies/apr/libapr-1.lib)
  endif (USE_APR)

else ()

  if (USE_PORTAUDIO)
    pkg_search_module(PORTAUDIO REQUIRED portaudio-2.0)
  endif (USE_PORTAUDIO)

  if (USE_APR)
    pkg_search_module(APR REQUIRED apr-1)
  endif (USE_APR)
  
endif (WINDOWS)

if (USE_PORTAUDIO)
	# add_definitions(-DAL_USE_PORTAUDIO)
	set(PORTAUDIO_DEFINITIONS -DAL_USE_PORTAUDIO)
endif()

if (USE_APR)
  set(APR_HEADERS
    ${al_path}/include/al/core/io/al_Socket.hpp
  )
  set(APR_SOURCES
    src/core/io/al_SocketAPR.cpp
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