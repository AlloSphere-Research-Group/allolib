
#list(APPEND CMAKE_MODULE_PATH
#${CMAKE_CURRENT_LIST_DIR}
#)

if (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/serial/include/serial/serial.h")
  message("Serial library not found. Clone repo https://github.com/wjwwood/serial in this extension's directory.")
else()
#  list(APPEND ADDITIONAL_INCLUDE_DIRS ${ASSIMP_INCLUDE_DIR})
#  list(APPEND ADDITIONAL_LIBRARIES ${ASSIMP_LIBRARY})

set(serial_SRCS
    "${CMAKE_CURRENT_LIST_DIR}/serial/src/serial.cc"
    "${CMAKE_CURRENT_LIST_DIR}/serial/include/serial/serial.h"
    "${CMAKE_CURRENT_LIST_DIR}/serial/include/serial/v8stdint.h"
)

if (AL_MACOS)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)
    list(APPEND serial_SRCS "${CMAKE_CURRENT_LIST_DIR}/serial/src/impl/unix.cc")
    list(APPEND serial_SRCS "${CMAKE_CURRENT_LIST_DIR}/serial/src/impl/list_ports/list_ports_osx.cc")
elseif (AL_WINDOWS)
    list(APPEND serial_SRCS "${CMAKE_CURRENT_LIST_DIR}/serial/src/impl/win.cc")
    list(APPEND serial_SRCS "${CMAKE_CURRENT_LIST_DIR}/serial/src/impl/list_ports/list_ports_win.cc")

else(AL_LINUX)
    list(APPEND serial_SRCS "${CMAKE_CURRENT_LIST_DIR}/serial/src/impl/unix.cc")
    list(APPEND serial_SRCS "${CMAKE_CURRENT_LIST_DIR}/serial/src/impl/list_ports/list_ports_linux.cc")
endif()

  set(THIS_EXTENSION_SRC
    "${CMAKE_CURRENT_LIST_DIR}/src/al_Serial.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/al_Arduino.cpp"
    ${serial_SRCS}
  )

  set(THIS_EXTENSION_HEADERS
    "${CMAKE_CURRENT_LIST_DIR}/al_Serial.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/al_Arduino.hpp"
  )

  set(THIS_EXTENSION_LIBRARIES
  )

  set(THIS_EXTENSION_LIBRARY_NAME al_serial)

  add_library(${THIS_EXTENSION_LIBRARY_NAME} ${THIS_EXTENSION_SRC} ${THIS_EXTENSION_HEADERS})

  target_include_directories(${THIS_EXTENSION_LIBRARY_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/serial/include")
if (AL_MACOS)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)
    target_link_libraries(${THIS_EXTENSION_LIBRARY_NAME} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif (AL_WINDOWS)
    target_link_libraries(${THIS_EXTENSION_LIBRARY_NAME} setupapi)

else(AL_LINUX)
    target_link_libraries(${THIS_EXTENSION_LIBRARY_NAME} rt pthread)
endif()

  # c++14 It's very important to set this for the extension as not
  # setting it will cause targets upstream to not use c++14
  set_target_properties(${THIS_EXTENSION_LIBRARY_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    )

  target_link_libraries(${THIS_EXTENSION_LIBRARY_NAME} al ${THIS_EXTENSION_LIBRARIES})

  # Connections to outside
  set(CURRENT_EXTENSION_LIBRARIES ${THIS_EXTENSION_LIBRARY_NAME} ${THIS_EXTENSION_LIBRARIES})
  set(CURRENT_EXTENSION_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR})
#  set(CURRENT_EXTENSION_DLLS)

  set(CURRENT_EXTENSION_EXAMPLES
    "${CMAKE_CURRENT_LIST_DIR}/examples/serialin.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/examples/arduino.cpp"
    )

#  message("libs: ${CURRENT_EXTENSION_LIBRARIES}")

endif()
