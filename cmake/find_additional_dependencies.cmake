# sets:
#   ADDITIONAL_INCLUDE_DIRS
#   ADDITIONAL_LIBRARIES
# 	ADDITIONAL_HEADERS
# 	ADDITIONAL_SOURCES
# 	ADDITIONAL_DEFINITIONS
# 	ADDITIONAL_COMPILE_FLAGS

#option(USE_APR "" OFF)
option(USE_MPI "" ON)

# al_path needs to be set prior to calling this script

  # if (USE_APR)
  #   find_package(PkgConfig REQUIRED)
  #   pkg_search_module(APR REQUIRED apr-1)
  # endif (USE_APR)

  # for freeimage, assimp, freetype
  list(APPEND CMAKE_MODULE_PATH
    ${al_path}/cmake/find_scripts
  )

  find_package(FreeImage QUIET)
  find_package(Assimp QUIET)
  find_package(Freetype QUIET)
  
if (AL_WINDOWS AND (NOT FREEIMAGE_FOUND))

  # This should be moved to the find script
  # if (USE_APR)
  #   set(APR_INCLUDE_DIRS ${al_path}/dependencies/apr/include)
  #   set(APR_LIBRARIES ${al_path}/dependencies/apr/libapr-1.lib)
  # endif (USE_APR)

  FIND_PATH(FREEIMAGE_INCLUDE_PATH FreeImage.h
    ${al_path}/windows/FreeImage/Dist/x64
    DOC "The directory where FreeImage.h resides")
  FIND_LIBRARY(FREEIMAGE_LIBRARY
    NAMES FreeImage freeimage
    PATHS ${al_path}/windows/FreeImage/Dist/x64
    DOC "The FreeImage library")

  IF (FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY)
    SET( FREEIMAGE_FOUND TRUE)
  ELSE (FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY)
    SET( FREEIMAGE_FOUND FALSE)
    set (FREEIMAGE_INCLUDE_PATH "")
    set (FREEIMAGE_LIBRARY "")
  ENDIF (FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY)

endif (AL_WINDOWS AND (NOT FREEIMAGE_FOUND))

if (AL_WINDOWS AND (NOT FREETYPE_FOUND))
	FIND_PATH( FREETYPE_INCLUDE_DIR ft2build.h
		${PROJECT_SOURCE_DIR}/windows/freetype2
		DOC "The directory where ft2build.h resides")
	FIND_LIBRARY( FREETYPE_LIBRARY
		NAMES freetype
		PATHS
		${PROJECT_SOURCE_DIR}/windows/freetype2
		DOC "The Freetype library")
		
message("Looking for freetype!!!!")
    SET(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIR})

endif (AL_WINDOWS AND (NOT FREETYPE_FOUND))

find_package(MPI QUIET)

# NOW ADD OPTIONAL FILES -------------------------------------------------------

# if (USE_APR)
#   list(APPEND ADDITIONAL_INCLUDE_DIRS ${APR_INCLUDE_DIRS})
#   list(APPEND ADDITIONAL_LIBRARIES ${APR_LIBRARIES})
#   list(APPEND ADDITIONAL_HEADERS ${al_path}/include/al/util/al_Socket.hpp)
#   list(APPEND ADDITIONAL_SOURCES ${al_path}/src/util/al_SocketAPR.cpp)
# endif(USE_APR)

if (FREEIMAGE_FOUND)
  if (AL_VERBOSE_OUTPUT)
    message("found freeimage")
  endif()
  list(APPEND ADDITIONAL_INCLUDE_DIRS ${FREEIMAGE_INCLUDE_PATH})
  list(APPEND ADDITIONAL_LIBRARIES ${FREEIMAGE_LIBRARY})
  list(APPEND ADDITIONAL_HEADERS ${al_path}/include/al/util/al_Image.hpp)
  list(APPEND ADDITIONAL_SOURCES ${al_path}/src/util/al_Image.cpp)
endif (FREEIMAGE_FOUND)

if (ASSIMP_LIBRARY)
  if (AL_VERBOSE_OUTPUT)
    message("found assimp")
  endif()
  list(APPEND ADDITIONAL_INCLUDE_DIRS ${ASSIMP_INCLUDE_DIR})
  list(APPEND ADDITIONAL_LIBRARIES ${ASSIMP_LIBRARY})
  list(APPEND ADDITIONAL_HEADERS ${al_path}/include/al/util/al_Asset.hpp)
  list(APPEND ADDITIONAL_SOURCES ${al_path}/src/util/al_Asset.cpp)
endif()

if (FREETYPE_INCLUDE_DIRS)
  if (AL_VERBOSE_OUTPUT)
    message("found freetype")
  endif()
  list(APPEND ADDITIONAL_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})
  list(APPEND ADDITIONAL_LIBRARIES ${FREETYPE_LIBRARY})
  list(APPEND ADDITIONAL_HEADERS ${al_path}/include/al/util/al_Font.hpp)
  list(APPEND ADDITIONAL_SOURCES ${al_path}/src/util/al_Font.cpp)
  message("Added!!")
endif()

if (USE_MPI AND MPI_CXX_FOUND)
  message("Using MPI: compiler ${MPI_C_COMPILER} ${MPI_CXX_INCLUDE_PATH}")
  set(MPI_DEFINITIONS "-DAL_BUILD_MPI")
  list(APPEND ADDITIONAL_INCLUDE_DIRS ${MPI_CXX_INCLUDE_PATH})
  list(APPEND ADDITIONAL_LIBRARIES ${MPI_CXX_LIBRARIES})
  list(APPEND ADDITIONAL_DEFINITIONS ${MPI_DEFINITIONS})
  list(APPEND ADDITIONAL_COMPILE_FLAGS ${MPI_CXX_COMPILE_FLAGS})
  list(APPEND ADDITIONAL_LINK_FLAGS ${MPI_CXX_LINK_FLAGS})
#  list(APPEND ADDITIONAL_HEADERS ${al_path}/include/al/util/al_Font.hpp)
#  list(APPEND ADDITIONAL_SOURCES ${al_path}/src/util/al_Font.cpp)
endif()
