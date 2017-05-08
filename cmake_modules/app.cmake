
include(${al_path}/cmake_modules/configure_platform.cmake)
# sets: MACOS || LINUX || WINDOWS_MINGW || WINDOWS, and PLATFORM_DEFINITION
include(${al_path}/cmake_modules/find_core_dependencies.cmake)
# sets: CORE_INCLUDE_DIRS, CORE_LIBRARIES, CORE_LIBRARY_DIRS
include(${al_path}/cmake_modules/find_additional_dependencies.cmake)
# sets: ADDITIONAL_INCLUDE_DIRS, ADDITIONAL_LIBRARIES, ADDITIONAL_HEADERS,
#       ADDITIONAL_SOURCES, ADDITIONAL_DEFINITIONS
include(${al_path}/cmake_modules/external.cmake)
# sets: EXTERNAL_LIBRARIES, EXTERNAL_DEFINITIONS
include(${al_path}/cmake_modules/basic_flags.cmake)
# sets: basic_flags

set(headers
  ${ADDITIONAL_HEADERS}
)

set(sources
  ${ADDITIONAL_SOURCES}
)

set(dirs_to_include
  ${al_path}/include
  ${CORE_INCLUDE_DIRS}
  ${ADDITIONAL_INCLUDE_DIRS}
)

set(libs_to_link
  ${CORE_LIBRARIES}
  ${ADDITIONAL_LIBRARIES}
  ${EXTERNAL_LIBRARIES}
)

set(definitions
  ${PLATFORM_DEFINITION}
  ${ADDITIONAL_DEFINITIONS}
  ${EXTERNAL_DEFINITIONS}
)

set(flags
  ${basic_flags}
)

link_directories(${CORE_LIBRARY_DIRS})

# --- setup app target ------------------------------------
add_executable(${app_name} ${app_files_list})

set_target_properties(${app_name} PROPERTIES DEBUG_POSTFIX _debug)

#paths
set_target_properties(${app_name}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${app_path}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${app_path}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${app_path}/bin
)

# flags
target_compile_options(${app_name} PUBLIC ${flags}) # public because of headers

# definitions
target_compile_definitions(${app_name} PRIVATE ${definitions})

# include dirs
target_include_directories(${app_name} PRIVATE ${dirs_to_include})

# libs
if (WINDOWS)
  target_link_libraries(${app_name} debug ${al_path}/al_debug.lib optimized ${al_path}/al.lib)
else()
  target_link_libraries(${app_name} debug ${al_path}/libal_debug.a optimized ${al_path}/libal.a)
endif (WINDOWS)
target_link_libraries(${app_name} ${libs_to_link})


if (WINDOWS)
  # when run from Visual Studio, working directory is where the solution is by default
  # set it to app output directory
  set_target_properties(${app_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${app_path}/bin)
  set_directory_properties(PROPERTIES VS_STARTUP_PROJECT ${app_name})

  # post build events for copying dlls
  set(post_build_command
    robocopy ${al_path}/dependencies/glew/bin/Release/x64 ${app_path}/bin glew32.dll &
    robocopy ${al_path}/dependencies/glfw/lib-vc2015 ${app_path}/bin glfw3.dll &
  )

  if (USE_PORTAUDIO)
  	list(APPEND post_build_command
  	  robocopy ${al_path}/dependencies/portaudio/ ${app_path}/bin portaudio_x64.dll &
  	)
  endif (USE_PORTAUDIO)

  if (USE_APR)
  	list(APPEND post_build_command
	  robocopy ${al_path}/dependencies/apr/ ${app_path}/bin libapr-1.dll &
  	)
  endif (USE_APR)

  list(APPEND post_build_command
  	IF %ERRORLEVEL% LEQ 1 exit 0
  )

  add_custom_command(
    TARGET ${app_name}
    POST_BUILD
    COMMAND ${post_build_command}
  )
endif (WINDOWS)