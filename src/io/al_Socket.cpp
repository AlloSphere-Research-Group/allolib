/*
TCP:
(PF_INET, PF_INET6), (SOCK_STREAM), (IPPROTO_TCP)
TCP Server:
1) Create a TCP socket
2) Bind socket to the listen port, with a call to bind()
3) Prepare socket to listen for connections with a call to listen()
4) Accepting incoming connections, via a call to accept()
5) Communicate with remote host, which can be done through, e.g., send()
6) Close each socket that was opened, once it is no longer needed, using close()

From
http://stackoverflow.com/questions/6189831/whats-the-purpose-of-using-sendto-recvfrom-instead-of-connect-send-recv-with-ud:

It is important to understand that TCP is "Connection Oriented" and UDP is
"Connection-less" protocol:

TCP: You need to connect first prior to sending/receiving data to/from a remote
host. UDP: No connection is required. You can send/receive data to/from any
host. Therefore, you will need to use sendto() on UDP socket in order to specify
the destination. Similarly, you would want to use recvfrom() to know where the
UDP data was received from.

You can actually use connect() on UDP socket as an option. In that case, you can
use send()/recv() on the UDP socket to send data to the address specified with
the connect() and to receive data only from the address. (The connect() on UDP
socket merely sets the default peer address and you can call connect() on UDP
socket as many times as you want, and the connect() on UDP socket, of course,
does not perform any handshake for connection.)
*/

#ifdef AL_WINDOWS

#else
#include <sys/ioctl.h>
#endif

#include "al/io/al_Socket.hpp"

//#include "al/system/al_Config.h"
#include "al/system/al_Printing.hpp"

using namespace al;

#if defined(AL_SOCKET_DUMMY)

class Socket::Impl {
public:
  Impl() {}
  Impl(uint16_t port, const char *address, float timeout_, int type) {}

  const std::string &address() const {
    static std::string s;
    return s;
  }
  uint16_t port() const { return 0; }
  float timeout() const { return 0; }

  bool open(uint16_t port, std::string address, float timeoutSec, int type) {
    return false;
  }
  void close() {}
  bool reopen() { return false; }
  bool bind() { return false; }
  bool connect() { return false; }
  void timeout(float v) {}
  bool listen() { return false; }
  bool accept(Socket::Impl *newSock) { return false; }
  bool opened() const { return false; }
  int recv(char *buffer, int maxlen, char *from) { return 0; }
  int send(const char *buffer, int len) { return 0; }
};

/*static*/ std::string Socket::hostIP() { return "0.0.0.0"; }
/*static*/ std::string Socket::hostName() { return "dummy_invalid"; }

// Native socket code
#else

// Windows and POSIX socket code match very closely, so we will just conform
// the differences with some new types.

#if defined(AL_WINDOWS)
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <string.h> // memset

// Initialization singleton
struct WsInit {
  WsInit() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (0 != iResult) {
      printf("WSAStartup failed: %d\n", iResult);
    }
  }
  ~WsInit() { WSACleanup(); }
  static WsInit &get() {
    static WsInit v;
    return v;
  }
};

const char *errorString() {
  static char s[4096];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, WSAGetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), s, sizeof(s),
                nullptr);
  return s;
}

#define INIT_SOCKET WsInit::get()
typedef SOCKET SocketHandle;
#define SHUT_RDWR SD_BOTH
DWORD secToTimeout(float t) {
  return t >= 0. ? DWORD(t * 1000. + 0.5) : 4294967295; // msec
}

#else // POSIX

#include <arpa/inet.h> // inet_ntoa
#include <errno.h>
#include <netdb.h>  // gethostbyname
#include <string.h> // memset, strerror
#include <sys/socket.h>
#include <sys/time.h> // timeval
#include <unistd.h>   // close, gethostname

#include <sstream>

const char *errorString() { return strerror(errno); }

#define INIT_SOCKET
typedef int SocketHandle;
timeval secToTimeout(float t) {
  if (t < 0)
    t = 2147483520.; // largest representable 32-bit int
  timeval tv;
  tv.tv_sec = int(t);
  tv.tv_usec = (t - tv.tv_sec) * 1000000;
  return tv;
}

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket ::close

#endif

struct Socket::Impl {
public:
  Impl() { INIT_SOCKET; }

  Impl(uint16_t port, const char *address, float timeout_, int type) : Impl() {
    // opens the socket also:
    if (!open(port, address, timeout_, type)) {
      AL_WARN("Socket::Impl failed to open port %d / address \"%s\"\n", port,
              address);
    }
  }

  const std::string &address() const { return mAddress; }
  uint16_t port() const { return mPort; }
  float timeout() const { return mTimeout; }

  bool open(uint16_t port, std::string address, float timeoutSec, int type) {
    close();

    mPort = port;
    mAddress = address;
    mType = type;

    int sockProto = type & 127;
    switch (sockProto) {
    case TCP:
      sockProto = IPPROTO_TCP;
      break;
    case UDP:
      sockProto = IPPROTO_UDP;
      break;
    case SCTP:
#ifdef IPPROTO_SCTP
      sockProto = IPPROTO_SCTP;
      break;
#else
      AL_WARN("Socket::SCTP not supported on this platform.");
      return false;
#endif
    default:;
    }

    int sockType = type & (127 << 8);
    switch (sockType) {
    case STREAM:
      sockType = SOCK_STREAM;
      break;
    case DGRAM:
      sockType = SOCK_DGRAM;
      break;
    case 0: // unspecified; choose sensible default, if possible
      switch (sockProto) {
      case TCP:
        sockType = SOCK_STREAM;
        break;
      case UDP:
      case SCTP:
        sockType = SOCK_DGRAM;
        break;
      default:;
      }
    }

    int sockFamily = type & (127 << 16);
    switch (sockFamily) {
    case 0:
    case INET:
      sockFamily = AF_INET;
      break;
    case INET6:
#ifdef AF_INET6
      sockFamily = AF_INET6;
      break;
#else
      AL_WARN("Socket::INET6 not supported on this platform.");
      return false;
#endif
    default:;
    }
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = sockFamily;
    hints.ai_socktype = sockType;
    hints.ai_protocol = sockProto;
    // FIXME set
    //    hints.ai_flags = AI_PASSIVE;

    char addr[16] = "\0";
    if (mAddress.empty() ||
        mAddress == "0.0.0.0") { // assume this means it's a server
                                 //      addr = nullptr;
      hints.ai_flags = AI_PASSIVE;
    }
    strncpy(addr, mAddress.c_str(), 16);

    // Resolve address and port
    char portAsString[5 + 1] = {0}; // max port number 65535
    {
      auto s = std::to_string(mPort);
      s.copy(portAsString, s.size());
    }

    if (0 != getaddrinfo(addr, portAsString, &hints, &mAddrInfo)) {
      AL_WARN("failed to resolve %s:%i: %s", address.c_str(), port,
              errorString());
      close();
      return false;
    }

#if 0
    {
      // Retrieve each address and print out the hex bytes
      int i = 0;
      for (auto ptr = mAddrInfo; ptr != NULL; ptr = ptr->ai_next) {

        printf("getaddrinfo response %d\n", i++);
        printf("\tFlags: 0x%x\n", ptr->ai_flags);
        printf("\tFamily: ");
        struct sockaddr_in *sockaddr_ipv4;
        LPSOCKADDR sockaddr_ip;
        char ipstringbuffer[46];
        DWORD ipbufferlength = 46;
        int iRetval = 0;
        switch (ptr->ai_family) {
        case AF_UNSPEC:
          printf("Unspecified\n");
          break;
        case AF_INET:
          printf("AF_INET (IPv4)\n");
          sockaddr_ipv4 = (struct sockaddr_in *)ptr->ai_addr;
          printf("\tIPv4 address %s\n", inet_ntoa(sockaddr_ipv4->sin_addr));
          break;
        case AF_INET6:
          printf("AF_INET6 (IPv6)\n");
          // the InetNtop function is available on Windows Vista and later
          // sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
          // printf("\tIPv6 address %s\n",
          //    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer,
          //    46) );

          // We use WSAAddressToString since it is supported on Windows XP and
          // later
          sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
          // The buffer length is changed by each call to WSAAddresstoString
          // So we need to set it for each iteration through the loop for safety
          ipbufferlength = 46;
          iRetval = WSAAddressToString(sockaddr_ip, (DWORD)ptr->ai_addrlen,
                                       NULL, ipstringbuffer, &ipbufferlength);
          if (iRetval)
            printf("WSAAddressToString failed with %u\n", WSAGetLastError());
          else
            printf("\tIPv6 address %s\n", ipstringbuffer);
          break;
        case AF_NETBIOS:
          printf("AF_NETBIOS (NetBIOS)\n");
          break;
        default:
          printf("Other %d\n", ptr->ai_family);
          break;
        }
        printf("\tSocket type: ");
        switch (ptr->ai_socktype) {
        case 0:
          printf("Unspecified\n");
          break;
        case SOCK_STREAM:
          printf("SOCK_STREAM (stream)\n");
          break;
        case SOCK_DGRAM:
          printf("SOCK_DGRAM (datagram) \n");
          break;
        case SOCK_RAW:
          printf("SOCK_RAW (raw) \n");
          break;
        case SOCK_RDM:
          printf("SOCK_RDM (reliable message datagram)\n");
          break;
        case SOCK_SEQPACKET:
          printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
          break;
        default:
          printf("Other %ld\n", ptr->ai_socktype);
          break;
        }
        printf("\tProtocol: ");
        switch (ptr->ai_protocol) {
        case 0:
          printf("Unspecified\n");
          break;
        case IPPROTO_TCP:
          printf("IPPROTO_TCP (TCP)\n");
          break;
        case IPPROTO_UDP:
          printf("IPPROTO_UDP (UDP) \n");
          break;
        default:
          printf("Other %ld\n", ptr->ai_protocol);
          break;
        }
        printf("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
        printf("\tCanonical name: %s\n", ptr->ai_canonname);
      }
    }
#endif

    // printf("family=%d\n", mAddrInfo->ai_family);

    //    // Create socket
    mSocketHandle = socket(mAddrInfo->ai_family, mAddrInfo->ai_socktype,
                           mAddrInfo->ai_protocol);
    if (!opened()) {
      AL_WARN("failed to create socket at %s:%i: %s", address.c_str(), port,
              errorString());
      close();
      return false;
    }

    // Set timeout
    timeout(timeoutSec);

    return true;
  }

  void close() {
    if (mAddrInfo) {
      freeaddrinfo(mAddrInfo);
      mAddrInfo = nullptr;
    }
    if (INVALID_SOCKET != mSocketHandle) {
      shutdown(mSocketHandle, SHUT_RDWR);
      closesocket(mSocketHandle);
      mSocketHandle = INVALID_SOCKET;
    }
  }

  bool reopen() { return open(mPort, mAddress, mTimeout, mType); }

  bool bind() { // for server-side
    if (opened()) {
      struct addrinfo *p;
      for (p = mAddrInfo; p != nullptr; p = p->ai_next) {
        if ((mSocketHandle = socket(p->ai_family, p->ai_socktype,
                                    p->ai_protocol)) == SOCKET_ERROR) {
          continue;
        }
        // int enable = 1;
        // // SO_REUSEADDR: "The rules used in validating addresses supplied to
        // // bind should allow reuse of local addresses."
        // if (SOCKET_ERROR == ::setsockopt(mSocketHandle, SOL_SOCKET,
        //                                  SO_REUSEADDR, &enable, sizeof(int)))
        //                                  {
        //   AL_WARN("unable to set SO_REUSEADDR on socket at %s:%i: %s",
        //           mAddress.c_str(), mPort, errorString());
        // }

        if (::bind(mSocketHandle, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR) {
          shutdown(mSocketHandle, SHUT_RDWR);
          closesocket(mSocketHandle);
          mSocketHandle = INVALID_SOCKET;
          //          printf("Error binding\n");
          continue;
        }

        // success!
        return true;
      }
    }
    return false;
  }

  bool connect() { // for client-side
    if (opened()) {
      if (SOCKET_ERROR == ::connect(mSocketHandle, mAddrInfo->ai_addr,
                                    (int)mAddrInfo->ai_addrlen)) {
        AL_WARN("unable to connect socket at %s:%i: %s", mAddress.c_str(),
                mPort, errorString());
        close();
        return false;
      }
    }
    return true;
  }

  void timeout(float v) {
    mTimeout = v;
    auto to = secToTimeout(v);
    for (auto opt : {SO_SNDTIMEO, SO_RCVTIMEO}) {
      if (mTimeout < 0) { // necessary?
#ifdef AL_WINDOWS
        decltype(WSABUF::len) nb = 0;
        ::ioctlsocket(mSocketHandle, FIONBIO, &nb);
#else
        int nb = 0;
        ::ioctl(mSocketHandle, FIONBIO, &nb);
#endif
      } else if (SOCKET_ERROR == ::setsockopt(mSocketHandle, SOL_SOCKET, opt,
                                              (char *)&to, sizeof(to))) {
        AL_WARN("unable to set timeout on socket at %s:%i: %s",
                mAddress.c_str(), mPort, errorString());
      }
    }
  }

  bool listen() {
    if (SOCKET_ERROR == ::listen(mSocketHandle, SOMAXCONN)) {
      AL_WARN("unable to listen at %s:%i: %s", mAddress.c_str(), mPort,
              errorString());
      return false;
    }
    return true;
  }

  bool accept(Socket::Impl *newSock) {
    // TODO
    /*struct sockaddr newSockAddr;
    SocketHandle newSOCKET = ::accept(mSocket, &newSockAddr);
    if(INVALID_SOCKET == newSOCKET){
            AL_WARN("unable to accept using listener at %s:%i: %S",
    mAddress.c_str(), mPort, errorString()); return false;
    }
    newSock->mSocket = newSOCKET;
    newSock->mAddrInfo.ai_family = newSockAddr.sa_family;
    switch(newSockAddr.sa_family){
    case AF_INET:
            newSock->mAddrInfo.ai_port = ((sockaddr_in
    *)&newSockAddr)->sin_port; newSock->mAddrInfo.ai_port = ((sockaddr_in
    *)&newSockAddr)->sin_port;
    }
    newSock->close();
    // Inherit timeout from parent
    newSock->timeout(mTimeout);
    return true;*/
    //    int pl_one, pl_two;
    socklen_t pl_one_len;
    struct sockaddr_in pl_one_addr;
    pl_one_len = sizeof(pl_one_addr);
    newSock->mSocketHandle =
        ::accept(mSocketHandle, (struct sockaddr *)&pl_one_addr, &pl_one_len);
    if (newSock->mSocketHandle == SOCKET_ERROR) {
      return false;
    }
    char address[64];
    switch (pl_one_addr.sin_family) {
    case AF_INET:
      inet_ntop(AF_INET, &(((struct sockaddr_in *)&pl_one_addr)->sin_addr),
                address, 64);
      break;

      //      case AF_INET6:
      //        inet_ntop(AF_INET6, &(((struct sockaddr_in6
      //        *)&pl_one_addr)->sin6_addr),
      //                  address, 64);
      //        break;

    default:
      strncpy(address, "Unknown AF", 64);
      return false;
    }
    newSock->mAddress = address;
    newSock->mPort = pl_one_addr.sin_port;
    return true;
  }

  bool opened() const { return INVALID_SOCKET != mSocketHandle; }

  size_t recv(char *buffer, size_t maxlen, char *from) {
    return ::recv(mSocketHandle, buffer, maxlen, 0);
  }

  int send(const char *buffer, int len) {
    return (int)::send(mSocketHandle, buffer, len, 0);
  }

private:
  int mType = 0;
  float mTimeout = -1;
  std::string mAddress;
  uint16_t mPort = 0;
  struct addrinfo *mAddrInfo = nullptr;
  SocketHandle mSocketHandle = INVALID_SOCKET;
};

/*static*/ std::string Socket::hostIP() {
  hostent *host = gethostbyname(hostName().c_str());
  if (nullptr == host) {
    AL_WARN("unable to obtain host IP: %s", errorString());
    return "";
  }

  // Note there can be more than one IP address. We loop just to demonstrate how
  // to get all the IPs, even though we return the first.
  int i = 0;
  while (host->h_addr_list[i] != 0) {
    struct in_addr addr;
    addr.s_addr = *(u_long *)host->h_addr_list[i++];
    return inet_ntoa(addr);
  }
  return "";
}

std::string Socket::nameToIp(std::string name) {
  hostent *record = gethostbyname(name.c_str());
  if (record == NULL) {
    printf("Could not resolve: %s is unavailable\n", name.c_str());
    return name;
  }
  in_addr *address = (in_addr *)record->h_addr;
  std::string ip_address = inet_ntoa(*address);
  return ip_address;
}

/*static*/ std::string Socket::hostName() {
  INIT_SOCKET;
  char buf[256] = {0};
  if (SOCKET_ERROR == gethostname(buf, sizeof(buf))) {
    AL_WARN("unable to obtain host name: %s", errorString());
  }
  return buf;
}

#endif // native socket

// Everything below is common across all platforms

Socket::Socket() : mImpl(new Impl) {}

Socket::Socket(uint16_t port, const char *address, al_sec timeout, int type)
    : mImpl(new Impl(port, address, timeout, type)) {}

Socket::~Socket() {
  close();
  delete mImpl;
}

const std::string &Socket::address() const { return mImpl->address(); }

bool Socket::opened() const { return mImpl->opened(); }

uint16_t Socket::port() const { return mImpl->port(); }

al_sec Socket::timeout() const { return mImpl->timeout(); }

bool Socket::bind() { return mImpl->bind(); }

bool Socket::connect() { return mImpl->connect(); }

void Socket::close() { mImpl->close(); }

bool Socket::open(uint16_t port, const char *address, al_sec timeout,
                  int type) {
  std::string addressChecked;
  if (address) {
    mValueSource.ipAddr = address;
    mValueSource.port = port;
    addressChecked = address;
  }
  return mImpl->open(port, addressChecked, timeout, type) && onOpen();
}

void Socket::timeout(al_sec v) { mImpl->timeout(v); }

size_t Socket::recv(char *buffer, size_t maxlen, char *from) {
  return mImpl->recv(buffer, maxlen, from);
}

size_t Socket::send(const char *buffer, size_t len) {
  return mImpl->send(buffer, len);
}

bool Socket::listen() { return mImpl->listen(); }

bool Socket::accept(Socket &sock) {
  bool accepted = mImpl->accept(sock.mImpl);
  if (accepted) {
    sock.mValueSource.ipAddr = sock.address();
    sock.mValueSource.port = sock.port();
  }
  return accepted;
}

bool SocketClient::onOpen() { return connect(); }

bool SocketServer::onOpen() { return bind(); }

ValueSource *Socket::valueSource() { return &mValueSource; }
