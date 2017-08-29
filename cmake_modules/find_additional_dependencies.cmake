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

  list(APPEND CMAKE_MODULE_PATH
    ${al_path}/cmake_modules/find_scripts
  )

  find_package(PkgConfig REQUIRED)

  if (USE_APR)
    pkg_search_module(APR REQUIRED apr-1)
  endif (USE_APR)

  find_package(FreeImage) # uses cmake_modules/find_scripts/FindFreeImage.cmake
                          # and sets:
                          #     FREEIMAGE_FOUND
                          #     FREEIMAGE_INCLUDE_PATH
                          #     FREEIMAGE_LIBRARY
  
endif (AL_WINDOWS)

if (USE_APR)
  set(APR_HEADERS
    ${al_path}/include/al/util/al_Socket.hpp
  )
  set(APR_SOURCES
    src/util/al_SocketAPR.cpp
  )
endif(USE_APR)

if (FREEIMAGE_FOUND)
  set(FREEIMAGE_HEADERS
    ${al_path}/include/al/util/al_Image.hpp
  )
  set(FREEIMAGE_SOURCES
    ${al_path}/src/util/al_Image.cpp
  )
endif (FREEIMAGE_FOUND)

set(ADDITIONAL_INCLUDE_DIRS
	# ${PORTAUDIO_INCLUDE_DIRS}
	${APR_INCLUDE_DIRS}
  ${FREEIMAGE_INCLUDE_PATH}
)

set(ADDITIONAL_LIBRARIES
	# ${PORTAUDIO_LIBRARIES}
	${APR_LIBRARIES}
  ${FREEIMAGE_LIBRARY}
)

set(ADDITIONAL_HEADERS
  ${FREEIMAGE_HEADERS}
	${APR_HEADERS}
)

set(ADDITIONAL_SOURCES
  ${FREEIMAGE_SOURCES}
	${APR_SOURCES}
)

set(ADDITIONAL_DEFINITIONS
	# ${PORTAUDIO_DEFINITIONS}
)
