# sets:
#   GAMMA_INCLUDE_DIR

set(GAMMA_INCLUDE_DIR ${al_path}/external/Gamma)

if (BUILDING_ALLOLIB)
set(GAMMA_NO_IO 1)
add_subdirectory(${al_path}/external/Gamma)
endif()

set(GAMMA_LIBRARY ${al_path}/build/lib/libGamma.a)
# set(GAMMA_LIBRARY Gamma)
