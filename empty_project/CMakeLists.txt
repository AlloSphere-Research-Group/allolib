cmake_minimum_required(VERSION 3.0)
set(app_path ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(app_name ${app_path} NAME)
project(${app_name})

# app config user specified
include(app_config.cmake) 

# if al_path not defined by run script, set to one specified by user config
if (NOT al_path)
	set(al_path ${user_al_path})
endif (NOT al_path)

include(${al_path}/cmake_modules/app.cmake)