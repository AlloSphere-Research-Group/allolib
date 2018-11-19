cmake_minimum_required(VERSION 3.0)

# This cmake file is for single cpp file projects that does not have
# CMakeLists.txt file in the project folder.

# user needs to pass definition `AL_APP_FILE` to this script

# example:
# at/proj/folder > mkdir build
# at/proj/folder > cd build
# at/proj/folder/build > cmake -DAL_APP_FILE=../my_app.cpp path/to/allolib/cmake/single_file
# at/proj/folder/build > make
# at/proj/folder/build > cd ../bin
# at/proj/folder/build > ./my_app

# allolib's run.sh script will do above process too

option(AL_VERBOSE_OUTPUT "" OFF)
option(AL_APP_RUN "" ON)

if (NOT AL_APP_FILE)
  message(FATAL_ERROR "\n[!] app file needs to be provided\n")
endif ()

if (IS_DIRECTORY AL_APP_FILE)
  message(FATAL_ERROR "\n[!] pass file to this cmake script\n")
endif ()


if (IS_ABSOLUTE ${AL_APP_FILE})
  get_filename_component(file_full_path ${AL_APP_FILE} ABSOLUTE)
else ()
  get_filename_component(file_full_path ${CMAKE_CURRENT_BINARY_DIR}/${AL_APP_FILE} ABSOLUTE)
endif ()

get_filename_component(app_name ${file_full_path} NAME_WE)
get_filename_component(app_path ${file_full_path} DIRECTORY)
set(app_files_list ${file_full_path})
set(al_path ${CMAKE_CURRENT_SOURCE_DIR}/../../../allolib)

if(EXISTS "${app_path}/flags.cmake")
  if (AL_VERBOSE_OUTPUT)
    message("found flags.cmake")
  endif()
  include(${app_path}/flags.cmake)
endif()

# TODO find cuttlebone
set(CUTTLEBONE_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/../../cuttlebone")
if (EXISTS ${CUTTLEBONE_SRC_DIR})
    set(CUTTLE_BUILD_TESTS_AND_EXAMPLES OFF CACHE BOOL "Build Cuttlebone extras" FORCE)
    add_subdirectory(${CUTTLEBONE_SRC_DIR} "${app_path}/build/cuttlebone")
    list(APPEND app_include_dirs ${CUTTLEBONE_SRC_DIR})
    list(APPEND app_link_libs cuttlebone)
    list(APPEND app_definitions -DAL_USE_CUTTLEBONE)
else()
    message("-- Cuttlebone not found. Not building Cuttlebone.")
endif()


if (AL_VERBOSE_OUTPUT)
	message("binary dir: ${CMAKE_CURRENT_BINARY_DIR}")
	message("file full path: ${file_full_path}")
	message("app path: ${app_path}")
	message("app name: ${app_name}")
	message("al path: ${al_path}")
	message("include dirs: ${app_include_dirs}")
	message("link libs: ${app_link_libs}")
  message("definitions: ${app_definitions}")
  message("compile flags: ${app_compile_flags}")
  message("linker flags: ${app_linker_flags}")
endif()

# inlcude allolib target and Gamma target
if (DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
  add_subdirectory("${al_path}" "${al_path}/build")
else()
	if (${CMAKE_BUILD_TYPE} MATCHES Debug)
    add_subdirectory("${al_path}" "${al_path}/build/Debug")
	else()
    add_subdirectory("${al_path}" "${al_path}/build/Release")
	endif()
endif ()

add_executable(${app_name} ${app_files_list})

set_target_properties(${app_name} PROPERTIES
  DEBUG_POSTFIX d
  RUNTIME_OUTPUT_DIRECTORY ${app_path}/bin
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${app_path}/bin
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${app_path}/bin
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

# flags
if (AL_WINDOWS)
  target_compile_options(${app_name} PRIVATE "")
else ()
  target_compile_options(${app_name} PRIVATE "-Wall")
endif (AL_WINDOWS)

get_target_property(al_libs al LINK_LIBRARIES)
target_link_libraries(${app_name} al  ${al_libs})

foreach(include_dir IN LISTS app_include_dirs)
    if (IS_ABSOLUTE ${include_dir})
        target_include_directories(${app_name} PRIVATE ${include_dir})
    else()
        target_include_directories(${app_name} PRIVATE ${app_path}/${include_dir})
    endif()
endforeach(include_dir IN app_include_dirs)

target_link_libraries(${app_name} ${app_link_libs})
target_compile_definitions(${app_name} PRIVATE ${app_definitions})
target_compile_options(${app_name} PRIVATE ${app_compile_flags})
# Item names starting with -, but not -l or -framework, are treated as linker flags.
target_link_libraries(${app_name} ${app_linker_flags})


if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (DEBUGGER "gdb" "-ex" "run")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (DEBUGGER "lldb" "-o run" "-ex ")
endif()

add_custom_target(${app_name}_run
    ./${app_name}
    DEPENDS ${app_name}
    WORKING_DIRECTORY ${app_path}/bin
    USES_TERMINAL
    )
add_custom_target(${app_name}_run_debug
    ${DEBUGGER} ./${app_name}d
    DEPENDS ${app_name}
    WORKING_DIRECTORY ${app_path}/bin
    USES_TERMINAL
    )

if (AL_WINDOWS)
  # when run from Visual Studio, working directory is where the solution is by default
  # set it to app output directory
  set_target_properties(${app_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${app_path}/bin)
  # startup project is `ALL_BUILD` by default so we change it to app project
  set_directory_properties(PROPERTIES VS_STARTUP_PROJECT ${app_name})

  if (SNDFILE_LIBRARY)
      #This is brought in from Gamma external library, but we might need to copy it to bin
      message("Copying ${SNDFILE_LIBRARY}" )
      get_filename_component(SNDFILE_LIBRARY_NAME "${SNDFILE_LIBRARY}" NAME_WE)
      get_filename_component(SNDFILE_LIBRARY_DIR "${SNDFILE_LIBRARY}" DIRECTORY)
      list(APPEND post_build_command
        robocopy "${SNDFILE_LIBRARY_DIR}/../bin" ${app_path}/bin ${SNDFILE_LIBRARY_NAME}.dll &
      )
  endif(SNDFILE_LIBRARY)

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

  if (FREEIMAGE_FOUND)
    list(APPEND post_build_command
      robocopy ${al_path}/dependencies/FreeImage/Dist/x64 ${app_path}/bin FreeImage.dll &
    )
  endif (FREEIMAGE_FOUND)

  list(APPEND post_build_command
    IF %ERRORLEVEL% LEQ 1 exit 0
  )

  add_custom_command(
    TARGET ${app_name}
    POST_BUILD
    COMMAND ${post_build_command}
  )

add_custom_command(
  TARGET ${app_name}_run
  POST_BUILD
  COMMAND ${post_build_command}
)
add_custom_command(
  TARGET ${app_name}_run_debug
  POST_BUILD
  COMMAND ${post_build_command}
)
endif (AL_WINDOWS)

