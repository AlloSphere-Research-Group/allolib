
### Oscpack ###

# add_subdirectory(external/oscpack)
# find_package(oscpack REQUIRED)
if (WIN32)
    set(IpSystemTypePath ip/win32)
else ()
    set(IpSystemTypePath ip/posix)
endif ()

add_library(oscpack ${AL_LIBRARY_TYPE}

    external/oscpack/ip/IpEndpointName.h
    external/oscpack/ip/IpEndpointName.cpp

    external/oscpack/ip/NetworkingUtils.h
    external/oscpack/${IpSystemTypePath}/NetworkingUtils.cpp

    external/oscpack/ip/UdpSocket.h
    external/oscpack/${IpSystemTypePath}/UdpSocket.cpp

    external/oscpack/ip/PacketListener.h
    external/oscpack/ip/TimerListener.h

    external/oscpack/osc/OscTypes.h
    external/oscpack/osc/OscTypes.cpp 
    external/oscpack/osc/OscHostEndianness.h
    external/oscpack/osc/OscException.h
    external/oscpack/osc/OscPacketListener.h
    external/oscpack/osc/MessageMappingOscPacketListener.h
    external/oscpack/osc/OscReceivedElements.h
    external/oscpack/osc/OscReceivedElements.cpp
    external/oscpack/osc/OscPrintReceivedElements.h
    external/oscpack/osc/OscPrintReceivedElements.cpp
    external/oscpack/osc/OscOutboundPacketStream.h
    external/oscpack/osc/OscOutboundPacketStream.cpp
)

target_include_directories(oscpack PUBLIC external/oscpack)

if(AL_WINDOWS)
  target_compile_definitions(oscpack PUBLIC NOMINMAX)
endif(AL_WINDOWS)

set_target_properties(oscpack PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    DEBUG_POSTFIX d
)

if (WIN32)
    target_link_libraries(oscpack PUBLIC Ws2_32 winmm)
endif ()




### alprotocol ###

set(headers
  include/al/protocol/al_OSC.hpp
  include/al/protocol/al_CommandConnection.hpp
)

set(sources
  src/protocol/al_OSC.cpp
  src/protocol/al_CommandConnection.cpp
)

add_library(alprotocol ${AL_LIBRARY_TYPE} ${headers} ${sources})

set_target_properties(alprotocol PROPERTIES
  DEBUG_POSTFIX d
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

target_include_directories(alprotocol PUBLIC
  include
)

target_link_libraries(alprotocol PUBLIC
  alio
  alsystem
  oscpack
)