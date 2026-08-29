# serial has a cmake build file but requires catkin, so we build it here.

set(serial_src_files
  "${CMAKE_CURRENT_LIST_DIR}/external/serial/src/serial.cc"
  "${CMAKE_CURRENT_LIST_DIR}/external/serial/include/serial/serial.h"
  "${CMAKE_CURRENT_LIST_DIR}/external/serial/include/serial/v8stdint.h"
)

if(AL_MACOS)
  find_library(IOKIT_LIBRARY IOKit)
  find_library(FOUNDATION_LIBRARY Foundation)
  list(APPEND serial_src_files "${CMAKE_CURRENT_LIST_DIR}/external/serial/src/impl/unix.cc")
  list(APPEND serial_src_files "${CMAKE_CURRENT_LIST_DIR}/external/serial/src/impl/list_ports/list_ports_osx.cc")
elseif(AL_WINDOWS)
  list(APPEND serial_src_files "${CMAKE_CURRENT_LIST_DIR}/external/serial/src/impl/win.cc")
  list(APPEND serial_src_files "${CMAKE_CURRENT_LIST_DIR}/external/serial/src/impl/list_ports/list_ports_win.cc")
elseif(AL_LINUX)
  list(APPEND serial_src_files "${CMAKE_CURRENT_LIST_DIR}/external/serial/src/impl/unix.cc")
  list(APPEND serial_src_files "${CMAKE_CURRENT_LIST_DIR}/external/serial/src/impl/list_ports/list_ports_linux.cc")
else()
  message(FATAL_ERROR "System platform variable not set properly")
endif()

add_library(serial ${serial_src_files})

target_include_directories(serial PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/external/serial/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(AL_MACOS)
  target_link_libraries(serial ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif(AL_WINDOWS)
  target_link_libraries(serial setupapi)
  target_compile_definitions(serial PUBLIC NOMINMAX)
elseif(AL_LINUX)
  target_link_libraries(serial rt pthread)
else()
  message(FATAL_ERROR "System platform variable not set properly")
endif()

set_target_properties(serial PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

### alserial ###

add_library(alserial ${AL_LIBRARY_TYPE})
add_library(al::serial ALIAS alserial)

target_sources(alserial
  PRIVATE
    src/io/al_SerialIO.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/io/al_SerialIO.hpp
)

al_configure_module(alserial EXPORT_NAME serial)

target_link_libraries(alserial PUBLIC
  al::types
  al::system
  serial
)

al_install_module(alserial)
