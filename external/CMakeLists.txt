set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries (general)" FORCE)
set(BUILD_TESTING     OFF CACHE BOOL "Build test targets (general)" FORCE)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "Build the GLFW test programs" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "Build the GLFW documentation" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "Generate installation target" FORCE)
set(GLFW_VULKAN_STATIC  OFF CACHE BOOL "Assume Vulkan loader linked with app" FORCE)
set(GLFW_USE_OSMESA     OFF CACHE BOOL "OSMesa for offscreen context" FORCE)
set(GLFW_USE_HYBRID_HPG OFF CACHE BOOL "Force high-performance GPU" FORCE)
set(GLFW_USE_WAYLAND    OFF CACHE BOOL "Use Wayland for window creation" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "MSVC runtime library DLL" FORCE)

set(RTAUDIO_BUILD_STATIC_LIBS ON  CACHE BOOL "" FORCE)
set(RTAUDIO_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(RTAUDIO_BUILD_TESTING     OFF CACHE BOOL "" FORCE)
set(RTAUDIO_TARGETNAME_UNINSTALL RTAUDIO_UNINSTALL CACHE STRING "" FORCE)

set(RTMIDI_BUILD_STATIC_LIBS ON  CACHE BOOL "" FORCE)
set(RTMIDI_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(RTMIDI_BUILD_TESTING     OFF CACHE BOOL "" FORCE)
set(RTMIDI_TARGETNAME_UNINSTALL RTMIDI_UNINSTALL CACHE STRING "" FORCE)

set(ENABLE_LIBCXX          OFF CACHE BOOL "use libc++ (cpptoml)" FORCE)
set(CPPTOML_BUILD_EXAMPLES OFF CACHE BOOL "examples (cpptoml)"   FORCE)

set(JSON_BuildTests OFF CACHE BOOL "tests (nlohmann_json)" FORCE)

set(BUILD_UTILS OFF CACHE BOOL "no utils for glew" FORCE)

add_subdirectory(Gamma)
add_subdirectory(glfw)
add_subdirectory(glad)
add_subdirectory(rtaudio)
add_subdirectory(rtmidi)
add_subdirectory(cpptoml)
add_subdirectory(json)
add_subdirectory(stb)
add_subdirectory(dr_libs)

# Imgui
add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
)

target_include_directories(imgui PUBLIC
    imgui
    imgui/examples
)

set_target_properties(imgui PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    DEBUG_POSTFIX d
)

# Oscpack
if (WIN32)
    set(IpSystemTypePath ip/win32)
else ()
    set(IpSystemTypePath ip/posix)
endif ()

add_library(oscpack STATIC

    oscpack/ip/IpEndpointName.h
    oscpack/ip/IpEndpointName.cpp

    oscpack/ip/NetworkingUtils.h
    oscpack/${IpSystemTypePath}/NetworkingUtils.cpp

    oscpack/ip/UdpSocket.h
    oscpack/${IpSystemTypePath}/UdpSocket.cpp

    oscpack/ip/PacketListener.h
    oscpack/ip/TimerListener.h

    oscpack/osc/OscTypes.h
    oscpack/osc/OscTypes.cpp 
    oscpack/osc/OscHostEndianness.h
    oscpack/osc/OscException.h
    oscpack/osc/OscPacketListener.h
    oscpack/osc/MessageMappingOscPacketListener.h
    oscpack/osc/OscReceivedElements.h
    oscpack/osc/OscReceivedElements.cpp
    oscpack/osc/OscPrintReceivedElements.h
    oscpack/osc/OscPrintReceivedElements.cpp
    oscpack/osc/OscOutboundPacketStream.h
    oscpack/osc/OscOutboundPacketStream.cpp
)

target_include_directories(oscpack PUBLIC oscpack)

set_target_properties(oscpack PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    DEBUG_POSTFIX d
)

if (WIN32)
    target_link_libraries(oscpack PUBLIC Ws2_32 winmm)
endif ()

# serial
# Even though serial has a cmake build file, it requires catkin...

set(serial_src_files
    "${CMAKE_CURRENT_LIST_DIR}/serial/src/serial.cc"
    "${CMAKE_CURRENT_LIST_DIR}/serial/include/serial/serial.h"
    "${CMAKE_CURRENT_LIST_DIR}/serial/include/serial/v8stdint.h"
)

if (AL_MACOS)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)
    list(APPEND serial_src_files "${CMAKE_CURRENT_LIST_DIR}/serial/src/impl/unix.cc")
    list(APPEND serial_src_files "${CMAKE_CURRENT_LIST_DIR}/serial/src/impl/list_ports/list_ports_osx.cc")
elseif (AL_WINDOWS)
    list(APPEND serial_src_files "${CMAKE_CURRENT_LIST_DIR}/serial/src/impl/win.cc")
    list(APPEND serial_src_files "${CMAKE_CURRENT_LIST_DIR}/serial/src/impl/list_ports/list_ports_win.cc")

else(AL_LINUX)
    list(APPEND serial_src_files "${CMAKE_CURRENT_LIST_DIR}/serial/src/impl/unix.cc")
    list(APPEND serial_src_files "${CMAKE_CURRENT_LIST_DIR}/serial/src/impl/list_ports/list_ports_linux.cc")
endif()

  add_library(serial ${serial_src_files})

  target_include_directories(serial PUBLIC "${CMAKE_CURRENT_LIST_DIR}/serial/include")
if (AL_MACOS)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)
    target_link_libraries(serial ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif (AL_WINDOWS)
    target_link_libraries(serial setupapi)

else(AL_LINUX)
    target_link_libraries(serial rt pthread)
endif()

  # c++14 It's very important to set this for the extension as not
  # setting it will cause targets upstream to not use c++14
  set_target_properties(serial PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    )

