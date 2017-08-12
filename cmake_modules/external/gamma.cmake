# sets:
#   GAMMA_INCLUDE_DIR

set(GAMMA_INCLUDE_DIR ${al_path}/external/Gamma)

if (BUILDING_ALLOLIB)
    add_subdirectory(${al_path}/external/Gamma)
endif()

if (WINDOWS)
    set(GAMMA_LIBRARY ${al_path}/build/lib/libGamma.lib)
else()
    set(GAMMA_LIBRARY ${al_path}/build/lib/libGamma.a)
endif (WINDOWS)