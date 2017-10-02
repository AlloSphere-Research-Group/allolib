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

  FIND_PATH(FREEIMAGE_INCLUDE_PATH FreeImage.h
    ${al_path}/dependencies/FreeImage/Dist/x64
    DOC "The directory where FreeImage.h resides")
  FIND_LIBRARY(FREEIMAGE_LIBRARY
    NAMES FreeImage freeimage
    PATHS ${al_path}/dependencies/FreeImage/Dist/x64
    DOC "The FreeImage library")

  IF (FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY)
    SET( FREEIMAGE_FOUND TRUE)
  ELSE (FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY)
    SET( FREEIMAGE_FOUND FALSE)
  ENDIF (FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY)

else ()

  if (USE_APR)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(APR REQUIRED apr-1)
  endif (USE_APR)

  list(APPEND CMAKE_MODULE_PATH
    ${al_path}/cmake_modules/find_scripts
  )

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
  message("found freeimage")
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
