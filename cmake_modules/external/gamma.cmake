# sets:
#   GAMMA_INCLUDE_DIR

set(GAMMA_INCLUDE_DIR ${al_path}/external/Gamma)

if (BUILDING_ALLOLIB)
    add_subdirectory(${al_path}/external/Gamma)
endif()

if (AL_WINDOWS)
    # set(GAMMA_LIBRARY ${al_path}/build/lib/Gamma.lib)
	set(GAMMA_DEBUG_LIBRARY ${al_path}/external/Gamma/lib/Gamma_debug.lib)
	set(GAMMA_RELEASE_LIBRARY ${al_path}/external/Gamma/lib/Gamma.lib)
else()
    # set(GAMMA_LIBRARY ${al_path}/build/lib/libGamma.a)
	set(GAMMA_DEBUG_LIBRARY ${al_path}/external/Gamma/lib/libGamma_debug.a)
	set(GAMMA_RELEASE_LIBRARY ${al_path}/external/Gamma/lib/libGamma.a)
endif (AL_WINDOWS)