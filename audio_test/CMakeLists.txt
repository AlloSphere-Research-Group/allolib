cmake_minimum_required(VERSION 3.0)
get_filename_component(app_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message(${app_name})
project(${app_name})

# CHECK PLATFORM ###########################################
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(MACOS 1)
  message("MACOS")
  add_definitions(-DAL_OSX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX 1)
  message("LINUX")
  add_definitions(-DAL_LINUX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  if (MSYS)
    set(WINDOWS_CYGWIN 1)
    message("WINDOWS_CYGWIN")
    # later change to AL_WINDOWS_MSYS?
    add_definitions(-DAL_WINDOWS)
  else () # WIN32 (NOTE: "MSVC" is defined at visual studio)
    set(WINDOWS 1)
    message("WINDOWS")
    add_definitions(-DAL_WINDOWS)
  endif()
else ()
endif ()

# FLAGS ####################################################

# set(CMAKE_BUILD_TYPE Debug)
if (WINDOWS)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin)
  set(CMAKE_CXX_FLAGS "/W2")
else ()
  set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wno-unused")
endif (WINDOWS)

# FIND LIBS ################################################

find_package(OpenGL REQUIRED)
if (WINDOWS)
  set(lib_path ${CMAKE_CURRENT_SOURCE_DIR}/../libs)
  set(GLFW_INCLUDE_DIRS ${lib_path}/glfw/include)
  set(GLFW_LIBRARIES ${lib_path}/glfw/glfw3dll.lib)
  set(GLEW_INCLUDE_DIRS ${lib_path}/glew/include)
  set(GLEW_LIBRARIES ${lib_path}/glew/glew32.lib)
  set(PORTAUDIO_INCLUDE_DIRS ${lib_path}/portaudio/include)
  set(PORTAUDIO_LIBRARIES ${lib_path}/portaudio/portaudio_x64.lib)
  set(APR_INCLUDE_DIRS ${lib_path}/apr/include)
  set(APR_LIBRARIES ${lib_path}/apr/libapr-1.lib)
else ()
  find_package(GLEW REQUIRED)
  find_package(PkgConfig REQUIRED)
  pkg_search_module(GLFW REQUIRED glfw3)
  pkg_search_module(PORTAUDIO REQUIRED portaudio-2.0)
  pkg_search_module(APR REQUIRED apr-1)
endif (WINDOWS)

add_executable(${app_name} main.cpp)
set_target_properties(${app_name} PROPERTIES DEBUG_POSTFIX _debug)

target_include_directories(${app_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
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
  target_link_libraries(${app_name} debug ${CMAKE_CURRENT_SOURCE_DIR}/../Debug/al.lib optimized  ${CMAKE_CURRENT_SOURCE_DIR}/../Release/al.lib)
else()
  target_link_libraries(${app_name} ${CMAKE_CURRENT_SOURCE_DIR}/../libal.a)
endif (WINDOWS)

# copy dlls
if (WINDOWS)
  set(post_build_command
    robocopy ${lib_path}/apr/ ${CMAKE_CURRENT_SOURCE_DIR}/bin libapr-1.dll &
    robocopy ${lib_path}/glew/ ${CMAKE_CURRENT_SOURCE_DIR}/bin glew32.dll &
    robocopy ${lib_path}/glfw/ ${CMAKE_CURRENT_SOURCE_DIR}/bin glfw3.dll &
    robocopy ${lib_path}/portaudio/ ${CMAKE_CURRENT_SOURCE_DIR}/bin portaudio_x64.dll &
    IF %ERRORLEVEL% LEQ 1 exit 0
  )

  add_custom_command(
    TARGET ${app_name}
    POST_BUILD
    COMMAND ${post_build_command}
  )
endif (WINDOWS)