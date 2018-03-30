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
set(al_path ${CMAKE_CURRENT_LIST_DIR}/../..)

option(AL_VERBOSE_OUTPUT "" OFF)

if (AL_VERBOSE_OUTPUT)
	message("binary dir: ${CMAKE_CURRENT_BINARY_DIR}")
	message("file full path: ${file_full_path}")
	message("app path: ${app_path}")
	message("app name: ${app_name}")
	message("al path: ${al_path}")
	message("include dirs: ${app_include_dirs}")
	message("link libs: ${app_link_libs}")
endif()

if(EXISTS "${app_path}/flags.cmake")
	if (AL_VERBOSE_OUTPUT)
		message("found flags.cmake")
	endif()
	include(${app_path}/flags.cmake)
endif()

# inlcude allolib target and Gamma target
if (DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../allolib ${CMAKE_CURRENT_LIST_DIR}/../../../allolib/build)
else()
	if (${CMAKE_BUILD_TYPE} MATCHES Debug)
		add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../allolib ${CMAKE_CURRENT_LIST_DIR}/../../../allolib/build/Debug)
	else()
		add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../allolib ${CMAKE_CURRENT_LIST_DIR}/../../../allolib/build/Release)
	endif()
endif ()

add_executable(${app_name} ${app_files_list})

set_target_properties(${app_name} PROPERTIES
    DEBUG_POSTFIX _debug
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

target_link_libraries(${app_name} al Gamma)

if (AL_WINDOWS)
  # when run from Visual Studio, working directory is where the solution is by default
  # set it to app output directory
  set_target_properties(${app_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${app_path}/bin)
  # startup project is `ALL_BUILD` by default so we change it to app project
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
endif (AL_WINDOWS)