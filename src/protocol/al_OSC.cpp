#include "al/protocol/al_OSC.hpp"

#include <ctype.h> // isgraph
#include <stdio.h> // printf
#include <string.h>

#include <iostream>

#include "al/system/al_Printing.hpp"
#include "ip/UdpSocket.h"
#include "osc/OscException.h"
#include "osc/OscOutboundPacketStream.h"
#include "osc/OscPacketListener.h"
#include "osc/OscReceivedElements.h"
#include "osc/OscTypes.h"

/*
Summary of OSC 1.0 spec from http://opensoundcontrol.org

The unit of transmission of OSC is an OSC Packet.

An OSC Packet consists of
  its contents
  a contiguous block of binary data
  its size, the number of 8-bit bytes that comprise the contents

The contents of an OSC packet must be either an OSC Message or an OSC Bundle.
The first byte of the packet's contents unambiguously distinguishes between
these two alternatives.
The size of an OSC packet is always a multiple of 4.

An OSC message consists of
  1) an OSC Address Pattern
  2) an OSC Type Tag String
  3) zero or more OSC Arguments

An OSC Address Pattern is an OSC-string beginning with the character '/'

An OSC Type Tag String is an OSC-string beginning with the character ',' (comma)
followed by a sequence of characters corresponding exactly to the sequence of
OSC Arguments in the given message.

i int32
f float32
s OSC-string
b OSC-blob

Anatomy of a bundle:

OSC-string "#bundle"      8 bytes       How to know that this data is a bundle
OSC-timetag           8 bytes (int64)   Time tag that applies to the entire
bundle Size of first bundle element  4 bytes (int32)   First bundle element 1st
bundle element's contents size of 1st bundle Size of second bundle element 4
bytes (int32)   Second bundle element 2nd bundle element's contents size of 2nd
bundle etc.                        Addtional bundle elements

*/

#define OSCTRY(msg, expr)                                                      \
  try {                                                                        \
    expr                                                                       \
  } catch (::osc::Exception & e) {                                             \
    AL_WARN("OSC error: %s", e.what());                                        \
  }

//#define VERBOSE
#ifdef VERBOSE
#define DPRINTF(...) printf(__VA_ARGS__)
#include <netinet/in.h> // for ntohl
#else
#define DPRINTF(...)
#endif

namespace al {
namespace osc {

class Packet::Impl : public ::osc::OutboundPacketStream {
public:
  Impl(char *buf, size_t size) : ::osc::OutboundPacketStream(buf, size) {}
};

Packet::Packet(int size) : mImpl(0), mData(size) {
  OSCTRY("Packet::Packet", mImpl = new Impl(&mData[0], size);)
}

Packet::Packet(const char *contents, size_t size) : mImpl(0), mData(size) {
  OSCTRY("Packet::Packet1", memcpy(&mData[0], contents, size);
         mImpl = new Impl(&mData[0], size);)
}

Packet::~Packet() { delete mImpl; }

Packet &Packet::operator<<(int v) {
  OSCTRY("Packet::<<int", (*mImpl) << (::osc::int32)v;)
  return *this;
}
Packet &Packet::operator<<(unsigned v) {
  OSCTRY("Packet::<<unsigned", (*mImpl) << (::osc::int32)v;)
  return *this;
}
Packet &Packet::operator<<(float v) {
  OSCTRY("Packet::<<float", (*mImpl) << v;)
  return *this;
}
Packet &Packet::operator<<(double v) {
  OSCTRY("Packet::<<double", (*mImpl) << v;)
  return *this;
}
Packet &Packet::operator<<(char v) {
  OSCTRY("Packet<<char", (*mImpl) << v;)
  return *this;
}
Packet &Packet::operator<<(const char *v) {
  OSCTRY("Packet::<<const char *", (*mImpl) << v;)
  return *this;
}
Packet &Packet::operator<<(const std::string &v) {
  OSCTRY("Packet::<< string", (*mImpl) << v.c_str();)
  return *this;
}
Packet &Packet::operator<<(const Blob &v) {
  OSCTRY("Packet::<<Blob", (*mImpl) << ::osc::Blob(v.data, v.size);)
  return *this;
}

Packet &Packet::beginMessage(const std::string &addr) {
  OSCTRY("Packet::beginMessage", (*mImpl) << ::osc::BeginMessage(addr.c_str());)
  return *this;
}

Packet &Packet::endMessage() {
  OSCTRY("Packet::endMessage", (*mImpl) << ::osc::EndMessage;) return *this;
}

Packet &Packet::beginBundle(TimeTag timeTag) {
  OSCTRY("Packet::beginBundle", (*mImpl) << ::osc::BeginBundle(timeTag);)
  return *this;
}

Packet &Packet::endBundle() {
  OSCTRY("Packet::endBundle", (*mImpl) << ::osc::EndBundle;) return *this;
}

Packet &Packet::clear() {
  OSCTRY("Packet::clear", mImpl->Clear();)
  return *this;
}

const char *Packet::data() const { return mImpl->Data(); }

bool Packet::isBundle() const {
  return ::osc::ReceivedPacket(data(), size()).IsBundle();
}
bool Packet::isMessage() const {
  return ::osc::ReceivedPacket(data(), size()).IsMessage();
}

void Packet::printRaw() const {
  for (size_t i = 0; i < size(); ++i) {
    unsigned char c = data()[i];
    printf("%2x ", c);
    isgraph(c) ? printf(" %c     ", c) : printf("       ");
    if ((i + 1) % 4 == 0)
      printf("\n");
  }
}

size_t Packet::size() const { return mImpl->Size(); }

class Message::Impl : public ::osc::ReceivedMessage {
public:
  Impl(const char *message, int size)
      : ::osc::ReceivedMessage(::osc::ReceivedPacket(message, size)),
        args(ArgumentStream()) {
    // printf("made an ::osc::ReceivedMessage out of message %p and size %d\n",
    // message, size);
  }

  template <class T> void operator>>(T &v) { OSCTRY("Message>>", args >> v;) }

  ::osc::ReceivedMessageArgumentStream args;
};

Message::Message(const char *message, int size, const TimeTag &timeTag,
                 const char *senderAddr)
    : mImpl(new Impl(message, size)), mTimeTag(timeTag) {
  OSCTRY("Message()", mAddressPattern = mImpl->AddressPattern();
         mTypeTags = mImpl->ArgumentCount() ? mImpl->TypeTags() : "";
         resetStream();)
  if (senderAddr != nullptr) {
    strncpy(mSenderAddr, senderAddr, 32);
  } else {
    mSenderAddr[0] = '\0';
  }
}

Message::~Message() { OSCTRY("~Message()", delete mImpl;) }

void Message::print() const {
  OSCTRY(
      "Message::print",
      printf("%s, %s %" AL_PRINTF_LL "d from %s\n", addressPattern().c_str(),
             typeTags().c_str(), timeTag(), mSenderAddr);

      ::osc::ReceivedMessageArgumentIterator it = mImpl->ArgumentsBegin();

      printf("\targs = ("); for (unsigned i = 0; i < typeTags().size(); ++i) {
        char tag = typeTags()[i];
        switch (tag) {
        case 'f': {
          float v = it->AsFloat();
          printf("%g", v);
        } break;
        case 'i': {
          long v = it->AsInt32();
          printf("%ld", v);
        } break;
        case 'h': {
          long long v = it->AsInt64();
          printf("%" AL_PRINTF_LL "d", v);
        } break;
        case 'c': {
          char v = it->AsChar();
          printf("'%c' (=%3d)", isprint(v) ? v : ' ', v);
        } break;
        case 'd': {
          double v = it->AsDouble();
          printf("%g", v);
        } break;
        case 's': {
          const char *v = it->AsString();
          printf("%s", v);
        } break;
        case 'b':
          printf("blob");
          break;
        default:
          printf("?");
        }
        if (i < (typeTags().size() - 1))
          printf(", ");
        ++it;
      } printf(")\n");)
}

Message &Message::resetStream() {
  mImpl->args = mImpl->ArgumentStream();
  return *this;
}
Message &Message::operator>>(int &v) {
  ::osc::int32 r = 0;
  OSCTRY("Message >> int", (*mImpl) >> r;)
  v = r;
  return *this;
}
Message &Message::operator>>(float &v) {
  OSCTRY("Message >> float", (*mImpl) >> v;)
  return *this;
}
Message &Message::operator>>(double &v) {
  OSCTRY("Message >> double", (*mImpl) >> v;)
  return *this;
}
Message &Message::operator>>(char &v) {
  OSCTRY("Message >> char", (*mImpl) >> v;)
  return *this;
}
Message &Message::operator>>(const char *&v) {
  OSCTRY("Message >> const char *", (*mImpl) >> v;)
  return *this;
}
Message &Message::operator>>(std::string &v) {
  const char *r = "";
  OSCTRY("Message >> string", (*mImpl) >> r;)
  v = r;
  return *this;
}
Message &Message::operator>>(Blob &v) {
  ::osc::Blob b;
  OSCTRY("Message >> Blob", (*mImpl) >> b;)
  v.data = b.data;
  v.size = b.size;
  return *this;
}

class Send::SocketSender {
public:
  UdpTransmitSocket transmitSocket;

  SocketSender(uint16_t port, const char *address)
      : transmitSocket{IpEndpointName{address, port}} {}

  size_t send(const char *data, std::size_t size) {
    transmitSocket.Send(data, size);
    return size;
  }
};

Send::Send() {}
Send::Send(int size) : Packet(size) {}

Send::Send(uint16_t port, const char *address, al_sec /*timeout*/, int size)
    : Packet(size) {
  open(port, address);
}

Send::~Send() = default; // put this here so dtor sees the dtor of impl class

bool Send::open(uint16_t port, const char *address) {
  try {
    // unique_ptr assignment releases and deletes previously owned object
    socketSender = std::make_unique<SocketSender>(port, address);
    mAddress = address;
    mPort = port;
  } catch (const std::runtime_error &e) {
    std::cout << "run time exception at Send::open: " << e.what() << std::endl;
    return false;
  }
  return true;
}

size_t Send::send() {
  size_t r = send(*this);
  OSCTRY("Packet::endMessage", Packet::clear();)
  return r;
}

size_t Send::send(const Packet &p) {
  size_t r = 0;
  OSCTRY("Packet::endMessage", r = socketSender->send(p.data(), p.size());)
  return r;
}

static void *recvThreadFunc(void *user) {
  Recv *r = static_cast<Recv *>(user);
  r->loop();
  return NULL;
}

class Recv::SocketReceiver : public ::osc::OscPacketListener {
public:
  UdpListeningReceiveSocket receiveSocket;
  Recv *recv;

  SocketReceiver(uint16_t port, Recv *r)
      : ::osc::OscPacketListener{},
        receiveSocket{IpEndpointName{IpEndpointName::ANY_ADDRESS, port}, this},
        recv{r} {}

  SocketReceiver(uint16_t port, const char *address, Recv *r)
      : ::osc::OscPacketListener{},
        receiveSocket{IpEndpointName{IpEndpointName{address, port}}, this},
        recv{r} {}

  virtual void ProcessMessage(const ::osc::ReceivedMessage &m,
                              const IpEndpointName &remoteEndpoint) override {}

  virtual void ProcessPacket(const char *data, int size,
                             const IpEndpointName &remoteEndpoint) override {
    char addr[IpEndpointName::ADDRESS_STRING_LENGTH];
    remoteEndpoint.AddressAsString(addr);
    recv->parse(data, size, addr);
  }

  void loop() { /*receiveSocket.RunUntilSigInt();*/
    receiveSocket.Run();
  }

  void stop() { receiveSocket.AsynchronousBreak(); }
};

Recv::Recv() : mBuffer(1024), mBackground(false) {}

Recv::Recv(uint16_t port, const char *address, al_sec timeout)
    : mBuffer(1024), mBackground(false) {
  open(port, address, timeout);
}

Recv::~Recv() { stop(); }

bool Recv::open(uint16_t port, const char *address, al_sec timeout) {
  mOpen = false;
  try {
    // unique pointer assignment releases and deletes previously owned object
    if (*address == '\0') {
      socketReceiver =
          std::make_unique<SocketReceiver>(port, "localhost", this);
    } else {
      socketReceiver = std::make_unique<SocketReceiver>(port, address, this);
    }

    mAddress = address;
    mPort = port;
  } catch (const std::runtime_error &e) {
    std::cout << "run time exception at Recv::open: " << e.what() << " "
              << address << ":" << port << std::endl;
    return false;
  }
  mOpen = true;
  return true;
}

int Recv::recv() { return 0; }

bool Recv::start() {
  mBackground = true;
  return mThread.start(recvThreadFunc, this);
}

void Recv::stop() {
  if (socketReceiver) {
    socketReceiver->stop();
    if (mBackground) {
      mThread.join();
      mBackground = false;
    }
  }
}

void Recv::parse(const char *packet, int size, const char *senderAddr) {
  std::memcpy(&mBuffer[0], packet, size);
  auto messages = parse(&mBuffer[0], size, 1, senderAddr);
  for (auto *handler : mHandlers) {
    for (auto m : messages) {
      handler->onMessage(*m);
    }
  }
}

void Recv::loop() { socketReceiver->loop(); }

bool Recv::portAvailable(uint16_t port, const char *address) {
  std::unique_ptr<SocketReceiver> socketReceiver;
  try {
    // unique pointer assignment releases and deletes previously owned object
    if (*address == '\0') {
      socketReceiver =
          std::make_unique<SocketReceiver>(port, "localhost", nullptr);
    } else {
      socketReceiver = std::make_unique<SocketReceiver>(port, address, nullptr);
    }

  } catch (const std::runtime_error &e) {
    std::cout << "run time exception at Recv::open: " << e.what() << " "
              << address << " : " << port << std::endl;
    return false;
  }
  return true;
}

std::vector<std::shared_ptr<Message>> Recv::parse(const char *packet, int size,
                                                  TimeTag timeTag,
                                                  const char *senderAddr) {
#ifdef VERBOSE
  int i = 1;
#endif
  std::vector<std::shared_ptr<Message>> messages;
  try {
    DPRINTF("PacketHandler::parse(size %d, packet %p)\n", size, packet);
    DPRINTF("Data to parse: ");
    for (int i = 0; i < size; ++i) {
      DPRINTF("%c", packet[i]);
    }
    DPRINTF("\n");

    // this is the only generic entry point for parsing packets
    ::osc::ReceivedPacket p(packet, size);

    DPRINTF("Just made an ::osc::ReceivedPacket that has contents %p and "
            "size %d\n",
            p.Contents(), (int)p.Size());

    // iterate through all the bundle elements (bundles or messages)
    if (p.IsBundle()) {
      DPRINTF("It's a bundle\n");
      // char *afterTimeTag = (char *)packet+16;  // "#bundle\0" plus
      // 8-byte time tag
      DPRINTF("First bundle element has size %d\n",
              ntohl(*((int *)(packet + 16)) /*firstBundleElementSize*/));

      ::osc::ReceivedBundle r(p);

      // DPRINTF("Just made an ::osc::ReceivedBundle that has time tag at
      // %p and %d elements\n", r.timeTag_, r.ElementCount() );

      for (auto it = r.ElementsBegin(); it != r.ElementsEnd(); ++it) {
        const ::osc::ReceivedBundleElement &e = *it;

        DPRINTF("Just made an ::osc::ReceivedBundleElement with contents %p "
                "and size %d\n",
                e.Contents(), (int)e.Size());
        DPRINTF("Parsing bundle element %d\n", i++);
        DPRINTF("Made an ::osc::ReceivedBundleElement out of the iterator.\n");
        DPRINTF("\tcontents: %p\n", e.Contents());
        DPRINTF("\tsize: %d\n", (int)e.Size());
        DPRINTF("\ttimeTag %lu\n", (unsigned long)r.TimeTag());
        DPRINTF("\tLet's try to parse it...\n");

        auto innerMessages =
            parse(e.Contents(), e.Size(), r.TimeTag(), senderAddr);
        messages.insert(messages.end(), innerMessages.begin(),
                        innerMessages.end());
      }
    } else if (p.IsMessage()) {
      DPRINTF("Parsing a message\n");
      messages.push_back(
          std::make_shared<Message>(packet, size, timeTag, senderAddr));
    }
  } catch (::osc::Exception &e) {
    AL_WARN("OSC error: %s", e.what());
  }
  return messages;
}

} // namespace osc
} // namespace al
