# ## serial ###
# add_subdirectory(external/serial)
# Even though serial has a cmake build file, it requires catkin...

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

target_include_directories(serial PUBLIC "${CMAKE_CURRENT_LIST_DIR}/external/serial/include")

if(AL_MACOS)
  find_library(IOKIT_LIBRARY IOKit)
  find_library(FOUNDATION_LIBRARY Foundation)
  target_link_libraries(serial ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif(AL_WINDOWS)
  target_link_libraries(serial setupapi)
elseif(AL_LINUX)
  target_link_libraries(serial rt pthread)
else()
  message(FATAL_ERROR "System platform variable not set properly")
endif()

# c++14 It's very important to set this for the extension as not
# setting it will cause targets upstream to not use c++14
set_target_properties(serial PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

# ## alserial ###
set(headers
  include/al/io/al_SerialIO.hpp
)

set(sources
  src/io/al_SerialIO.cpp
)

add_library(alserial ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(alserial PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(alserial PUBLIC
  include
)

target_link_libraries(alserial PUBLIC
  altypes
  alsystem
  serial
)