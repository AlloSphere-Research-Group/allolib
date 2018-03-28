# sets:
#   GAMMA_INCLUDE_DIR

set(GAMMA_INCLUDE_DIR ${al_path}/external/Gamma)

add_subdirectory(${al_path}/external/Gamma)

if (AL_WINDOWS)
    # set(GAMMA_LIBRARY ${al_path}/build/lib/Gamma.lib)
	set(GAMMA_DEBUG_LIBRARY Gamma)
	set(GAMMA_RELEASE_LIBRARY Gamma)
else()
    # set(GAMMA_LIBRARY ${al_path}/build/lib/libGamma.a)
	set(GAMMA_DEBUG_LIBRARY Gamma)
	set(GAMMA_RELEASE_LIBRARY Gamma)
endif (AL_WINDOWS)
