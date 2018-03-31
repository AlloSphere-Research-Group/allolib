# sets:
#   OSCPACK_INCLUDE_DIR
#   OSCPACK_SRC

set(OSCPACK_INCLUDE_DIR external/oscpack)

set(OSCPACK_SRC
  external/oscpack/osc/OscOutboundPacketStream.cpp
  external/oscpack/osc/OscPrintReceivedElements.cpp
  external/oscpack/osc/OscReceivedElements.cpp
  external/oscpack/osc/OscTypes.cpp
  external/oscpack/ip/IpEndpointName.cpp
)

set(OSCPACK_LINK_LIBRARIES "")

if (AL_WINDOWS)
  list(APPEND OSCPACK_SRC
    external/oscpack/ip/win32/NetworkingUtils.cpp
    external/oscpack/ip/win32/UdpSocket.cpp
  )
  list(APPEND OSCPACK_LINK_LIBRARIES
    winmm
    Ws2_32
  )
else ()
  list(APPEND OSCPACK_SRC
    external/oscpack/ip/posix/NetworkingUtils.cpp
    external/oscpack/ip/posix/UdpSocket.cpp
  )
endif (AL_WINDOWS)
