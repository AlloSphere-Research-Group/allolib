cmake_minimum_required(VERSION 3.0)
project(proj_al_test)

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
  if (CYGWIN)
    set(WINDOWS_CYGWIN 1)
    message("WINDOWS_CYGWIN")
    # later change to AL_WINDOWS_CYGWIN?
    add_definitions(-DAL_WINDOWS)
  else () # WIN32
    set(WINDOWS 1)
    message("WINDOWS")
    add_definitions(-DAL_WINDOWS)
  endif()
else ()
endif ()

# FLAGS ####################################################

set(CMAKE_BUILD_TYPE Debug)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
if (WINDOWS)
  set(CMAKE_CXX_FLAGS "/W2")
else ()
  set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wno-unused")
endif (WINDOWS)

# FIND LIBS ################################################

find_package(OpenGL REQUIRED)
if (WINDOWS)
  # use static libs if windows (for now)
  set(lib_path ${CMAKE_CURRENT_SOURCE_DIR}/../libs)
  set(GLFW_INCLUDE_DIRS ${lib_path}/glfw/include)
  set(GLFW_LIBRARIES ${lib_path}/glfw/lib-vc2015/glfw3.lib)
  set(GLEW_INCLUDE_DIRS ${lib_path}/glew/include)
  set(GLEW_LIBRARIES ${lib_path}/glew/lib/Release/x64/glew32s.lib)
  set(PORTAUDIO_INCLUDE_DIRS ${lib_path}/portaudio/include)
  set(PORTAUDIO_LIBRARIES ${lib_path}/portaudio/portaudio_static_x64.lib)
  # set(APR_INCLUDE_DIR
  #   ${lib_path}/apr/apr/include
  #   ${lib_path}/apr/apr-iconv/include
  #   ${lib_path}/apr/apr-util/include
  # )
  # set(APR_LIBRARY
  #   ${lib_path}/apr/apr/x64/LibD/apr-1.lib
  #   ${lib_path}/apr/apr/x64/LibD/aprapp-1.lib
  #   ${lib_path}/apr/apr-iconv/x64/LibD/apriconv-1.lib
  #   ${lib_path}/apr/apr-util/x64/LibD/aprutil-1.lib
  # )
  # set(APR_INCLUDE_DIR
  #   ${lib_path}/apr2/include
  # )
  # set(APR_LIBRARY
  #   ${lib_path}/apr2/lib/v110/x64/Debug/static/libapr-1.lib
  # )
  set(APR_INCLUDE_DIR
    ${lib_path}/apr3/include
  )
  set(APR_LIBRARY
    # ${lib_path}/apr3/apr-1.lib
    ${lib_path}/apr3/libapr-1.lib
  )
else ()
  find_package(GLEW REQUIRED)
  find_package(PkgConfig REQUIRED)
  pkg_search_module(GLFW REQUIRED glfw3)
  pkg_search_module(PORTAUDIO REQUIRED portaudio-2.0)
endif (WINDOWS)

add_executable(al_test main.cpp)

target_include_directories(al_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(al_test PRIVATE ${GLFW_INCLUDE_DIRS})
target_include_directories(al_test PRIVATE ${GLEW_INCLUDE_DIRS})
target_include_directories(al_test PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
target_include_directories(al_test PRIVATE ${APR_INCLUDE_DIR})

target_link_libraries(al_test ${OPENGL_LIBRARY})
target_link_libraries(al_test ${GLEW_LIBRARIES})
target_link_libraries(al_test ${GLFW_LIBRARIES})
target_link_libraries(al_test ${PORTAUDIO_LIBRARIES})
target_link_libraries(al_test ${APR_LIBRARY})

if (WINDOWS)
  target_link_libraries(al_test ${CMAKE_CURRENT_SOURCE_DIR}/../Debug/al.lib)
else()
  target_link_libraries(al_test ${CMAKE_CURRENT_SOURCE_DIR}/../libal.a)
endif (WINDOWS)

add_custom_command(TARGET al_test
                   POST_BUILD
                   COMMAND robocopy "${lib_path}/apr3/" "${CMAKE_CURRENT_SOURCE_DIR}/Debug" "libapr-1.dll"
                   COMMENT "Running SomeCommand")