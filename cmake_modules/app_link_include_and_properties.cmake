set_target_properties(${app_name} PROPERTIES DEBUG_POSTFIX _debug)

target_include_directories(${app_name} PRIVATE ${al_path}/include)
target_include_directories(${app_name} PRIVATE ${GLFW_INCLUDE_DIRS})
target_include_directories(${app_name} PRIVATE ${GLEW_INCLUDE_DIRS})
target_include_directories(${app_name} PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
target_include_directories(${app_name} PRIVATE ${APR_INCLUDE_DIRS})

target_link_libraries(${app_name} ${OPENGL_LIBRARY})
target_link_libraries(${app_name} ${GLEW_LIBRARIES})
target_link_libraries(${app_name} ${GLFW_LIBRARIES})
target_link_libraries(${app_name} ${PORTAUDIO_LIBRARIES})
target_link_libraries(${app_name} ${APR_LIBRARIES})

if (WINDOWS)
  target_link_libraries(${app_name} debug ${al_path}/al_debug.lib optimized ${al_path}/al.lib)
else()
  target_link_libraries(${app_name} debug ${al_path}/libal_debug.a optimized ${al_path}/libal.a)
endif (WINDOWS)