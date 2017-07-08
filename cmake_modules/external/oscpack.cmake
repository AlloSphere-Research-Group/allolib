# sets:
#   OSCPACK_INCLUDE_DIR
#   OSCPACK_SRC

set(OSCPACK_INCLUDE_DIR ${al_path}/external/oscpack)

set(OSCPACK_SRC
  external/oscpack/osc/OscOutboundPacketStream.cpp
  external/oscpack/osc/OscPrintReceivedElements.cpp
  external/oscpack/osc/OscReceivedElements.cpp
  external/oscpack/osc/OscTypes.cpp
  external/oscpack/ip/IpEndPointName.cpp
)

if (WINDOWS)
  list(APPEND OSCPACK_SRC
    external/oscpack/ip/win32/NetworkingUtils.cpp
    external/oscpack/ip/win32/UdpSocket.cpp
  )
else ()
  list(APPEND OSCPACK_SRC
    external/oscpack/ip/posix/NetworkingUtils.cpp
    external/oscpack/ip/posix/UdpSocket.cpp
  )
endif (WINDOWS)