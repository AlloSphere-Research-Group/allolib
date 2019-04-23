# - Try to find Assimp
# Once done this will define
#
#  ASSIMP_FOUND - system has Assimp
#  ASSIMP_INCLUDE_DIR - the Assimp include directory
#  ASSIMP_LIBRARIES - Link these to use Assimp
#

include(LibFindMacros)

if((NOT ASSIMP_INCLUDE_DIR) AND (NOT ASSIMP_LIBRARY))
  # Use pkg-config to get hints about paths
  libfind_pkg_check_modules(ASSIMP_PKGCONF libassimp)

  # Assume Assimp 4. Bionic Beaver (Ubuntu 18.04 and current homebrew bring in Assimp 4)
  find_path(ASSIMP_INCLUDE_DIR
    NAMES assimp/types.h
    PATHS ${ASSIMP_PKGCONF_INCLUDE_DIRS}
    /usr/include
    /usr/local/include
    /opt/local/include
    "c:/Program Files/Assimp/include"
    )

  if(ASSIMP_INCLUDE_DIR)
    message(STATUS "Assimp found")
  endif(ASSIMP_INCLUDE_DIR)



  # Finally the library itself
  find_library(ASSIMP_LIBRARY
    NAMES assimp assimp-vc140-mt
    PATHS ${ASSIMP_PKGCONF_LIBRARY_DIRS}
    "c:/Program Files/Assimp/lib/x64"
    )

  #/usr/include/assimp
  #/usr/local/include
  #/opt/local/include/assimp
  #/usr/local/Cellar/assimp/2.0.863/include/assimp

  # Set the include dir variables and the libraries and let libfind_process do the rest.
  # NOTE: Singular variables for this library, plural for libraries this this lib depends on.
  set(ASSIMP_PROCESS_INCLUDES ${ASSIMP_INCLUDE_DIR})
  set(ASSIMP_PROCESS_LIBS ${ASSIMP_LIBRARY})
  libfind_process(ASSIMP)

endif((NOT ASSIMP_INCLUDE_DIR) AND (NOT ASSIMP_LIBRARY))

