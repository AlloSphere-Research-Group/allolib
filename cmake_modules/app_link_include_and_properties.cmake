# app_name and al_path needs to be set before calling this script

set_target_properties(${app_name} PROPERTIES DEBUG_POSTFIX _debug)

# when run from Visual Studio, working directory is where the solution is.
# set it to app output directory
if (WINDOWS)
  set_target_properties(${app_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${app_path}/bin)
endif (WINDOWS)

if (WINDOWS)
  target_link_libraries(${app_name} debug ${al_path}/al_debug.lib optimized ${al_path}/al.lib)
else()
  target_link_libraries(${app_name} debug ${al_path}/libal_debug.a optimized ${al_path}/libal.a)
endif (WINDOWS)

target_include_directories(${app_name} PRIVATE ${al_path}/include)
target_include_directories(${app_name} PRIVATE ${GLFW_INCLUDE_DIRS})
target_include_directories(${app_name} PRIVATE ${GLEW_INCLUDE_DIRS})
target_include_directories(${app_name} PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
target_include_directories(${app_name} PRIVATE ${APR_INCLUDE_DIRS})

target_link_libraries(${app_name} ${OPENGL_LIBRARY})
target_link_libraries(${app_name} ${GLEW_LIBRARIES})
target_link_libraries(${app_name} ${GLFW_LIBRARIES})
# target_link_libraries(${app_name} ${GLFW_STATIC_LIBRARIES})
target_link_libraries(${app_name} ${PORTAUDIO_LIBRARIES})
target_link_libraries(${app_name} ${APR_LIBRARIES})

# rtaudio
if (LINUX)
	target_link_libraries(${app_name} pulse	pulse-simple)
endif (LINUX)
if (MACOS)
	target_link_libraries(${app_name} ${COREAUDIO_LIB} ${COREFOUNDATION_LIB} pthread)
endif (MACOS)
if (WINDOWS)
	target_link_libraries(${app_name} ole32 winmm ksuser uuid)
endif(WINDOWS)