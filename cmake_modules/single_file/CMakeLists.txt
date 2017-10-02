cmake_minimum_required(VERSION 3.6)

# This cmake file is for single file projects that does not have
# CMakeLists.txt file in the project folder.

# user needs to pass definition `AL_APP_FILE` to this script

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

# app.cmake needs:
#         app_name
#         app_path
#         app_files_list
#         app_include_dirs (skipped here)
#         app_link_libs (skipped here)
#         al_path

get_filename_component(app_name ${file_full_path} NAME_WE)
get_filename_component(app_path ${file_full_path} DIRECTORY)
set(app_files_list ${file_full_path})
set(al_path ${CMAKE_CURRENT_SOURCE_DIR}/../..)

message("binary dir: ${CMAKE_CURRENT_BINARY_DIR}")
message("file full path: ${file_full_path}")
message("app path: ${app_path}")
message("app name: ${app_name}")
message("al path: ${al_path}")

include(${al_path}/cmake_modules/app.cmake)