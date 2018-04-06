cmake_minimum_required(VERSION 3.0)
set(app_path ${CMAKE_CURRENT_LIST_DIR})
get_filename_component(app_name ${app_path} NAME)
project(${app_name})
option(AL_VERBOSE_OUTPUT "" OFF)

# app config user specified
include(app_config.cmake) 

set(allolib_dir ${CMAKE_CURRENT_LIST_DIR}/${allolib_directory})

if (AL_VERBOSE_OUTPUT)
	message("binary dir: ${CMAKE_CURRENT_BINARY_DIR}")
	message("app path: ${app_path}")
	message("app name: ${app_name}")
	message("allolib path: ${allolib_dir}")
	message("include dirs: ${app_include_dirs}")
	message("link libs: ${app_link_libs}")
endif()

# inlcude allolib target
if (DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
	add_subdirectory(${allolib_dir} ${allolib_dir}/build)
else()
	if (${CMAKE_BUILD_TYPE} MATCHES Debug)
		add_subdirectory(${allolib_dir} ${allolib_dir}/build/Debug)
	else()
		add_subdirectory(${allolib_dir} ${allolib_dir}/build/Release)
	endif()
endif ()

add_executable(bin ${app_files_list})

set_target_properties(bin PROPERTIES
    DEBUG_POSTFIX _debug
    RUNTIME_OUTPUT_DIRECTORY ${app_path}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${app_path}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${app_path}/bin
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

# flags
if (WINDOWS)
  target_compile_options(bin PRIVATE "")
else ()
  target_compile_options(bin PRIVATE "-Wall")
endif (WINDOWS)

target_link_libraries(bin al)

if (WINDOWS)
  # when run from Visual Studio, working directory is where the solution is by default
  # set it to app output directory
  set_target_properties(bin PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${app_path}/bin)
  # startup project is `ALL_BUILD` by default so we change it to app project
  set_directory_properties(PROPERTIES VS_STARTUP_PROJECT bin)
endif (WINDOWS)