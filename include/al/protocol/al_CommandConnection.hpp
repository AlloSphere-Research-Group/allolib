#ifndef COMMANDCONNECTION_HPP
#define COMMANDCONNECTION_HPP

/*	Allocore --
        Multimedia / virtual environment application class library

        Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012. The Regents of the University of California. All
   rights reserved.

        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions are
   met:

                Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

                Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer in the
                documentation and/or other materials provided with the
   distribution.

                Neither the name of the University of California nor the names
   of its contributors may be used to endorse or promote products derived from
                this software without specific prior written permission.

        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
        IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

        File description:
        Objects for reading, writing, sending, and receiving
        OSC (Open Sound Control) packets.

        File author(s):
        Lance Putnam, 2010, putnam.lance@gmail.com
        Graham Wakefield, 2010, grrrwaaa@gmail.com
        Keehong Youn, 2017, younkeehong@gmail.com
*/

#include <atomic>
#include <cinttypes>
#include <condition_variable>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "al/io/al_Socket.hpp"
#include "al/types/al_SingleRWRingBuffer.hpp"
#include "al/types/al_ValueSource.hpp"

namespace al {

class Message {
public:
  Message(uint8_t *message, size_t length) : mData(message), mSize(length) {}
  std::string getString() {
    size_t startIndex = mReadIndex;
    while (mReadIndex < mSize && mData[mReadIndex] != 0x00) {
      mReadIndex++;
    }
    std::string s;
    if (mReadIndex < mSize && mReadIndex != startIndex) {
      s.resize(mReadIndex - startIndex);
      s.assign((const char *)&mData[startIndex]);
    }
    mReadIndex++; // skip final null
    return s;
  }

  bool empty() { return mSize == 0 || mReadIndex == mSize; }

  uint8_t getByte() {
    uint8_t val = mData[mReadIndex];
    mReadIndex++;
    return val;
  }

  template <typename DataType> DataType get() {
    DataType val;
    memcpy(&val, &mData[mReadIndex], sizeof(DataType));
    mReadIndex += sizeof(DataType);
    return val;
  }

  uint32_t getUint32() {
    uint32_t val;
    memcpy(&val, &mData[mReadIndex], 4);
    mReadIndex += 4;
    return val;
  }

  std::vector<std::string> getVectorString() {
    std::vector<std::string> vs;
    uint8_t count = mData[mReadIndex];
    mReadIndex++;
    for (uint8_t i = 0; i < count; i++) {
      vs.push_back(getString());
    }
    return vs;
  }

  void print() {
    std::string s;
    if (mReadIndex < mSize && mSize > 0) {
      s.resize(mSize - mReadIndex);
      strncpy((char *)s.data(), (char *)&mData[mReadIndex], mSize - mReadIndex);
      std::cout << s << std::endl;
    }
  }

  uint8_t *data() { return mData + mReadIndex; }
  size_t size() { return mSize; }
  size_t remainingBytes() { return mSize - mReadIndex; }

  void pushReadIndex(size_t numBytes) {
    mReadIndex += numBytes;
    if (mReadIndex > mSize) {
      std::cerr << "Message pushed too far" << std::endl;
    }
  }
  void setReadIndex(size_t numBytes) { mReadIndex = numBytes; }

private:
  uint8_t *mData;
  const size_t mSize;
  size_t mReadIndex{0};
};

class CommandConnection {
public:
  typedef enum {
    HANDSHAKE = 1,
    HANDSHAKE_ACK,
    GOODBYE,
    GOODBYE_ACK,
    PING,
    PONG,
    COMMAND_QUIT,
    COMMAND_LAST_INTERNAL = 32,
  } InternalCommands;

  virtual bool start(uint16_t port, const char *addr) = 0;
  virtual void stop();

  /**
   * @brief sendMessage
   * @param message
   * @param dst
   * @return
   *
   * if dst is nullptr, the message is sent to all connected sockets
   * If src is not nullptr, the message will not be sent to it
   */
  virtual bool sendMessage(uint8_t *message, size_t length,
                           Socket *dst = nullptr, ValueSource *src = nullptr) {
    return true;
  }

  virtual void onReceiveError() {}

  void setVerbose(bool verbose) { mVerbose = verbose; }

protected:
  virtual bool processIncomingMessage(Message &message, Socket *src) {
    auto command = message.getByte();
    if (command == PONG) {
      if (mVerbose) {
        std::cout << __FILE__ << "Got pong for " << src->address() << ":"
                  << src->port() << std::endl;
      }
    } else if (command == GOODBYE) {
      std::cerr << __FILE__ << "Goodbye not implemented" << std::endl;
    } else if (command == HANDSHAKE) {
      std::cerr << __FILE__ << "Unexpected handshake received" << std::endl;
    }
    return false;
  };

  virtual void onConnection(Socket *newConnection){};

  uint16_t mVersion = 0,
           mRevision = 0; // Subclasses must set these to ensure compatibility

  typedef enum { SERVER, CLIENT, NONE } BarrierState;
  BarrierState mState{BarrierState::NONE};
  std::mutex mConnectionsLock;

  bool mRunning{false};
  std::vector<std::unique_ptr<std::thread>> mConnectionThreads;
  std::vector<std::unique_ptr<std::thread>> mDataThreads;
  std::vector<std::shared_ptr<al::Socket>>
      mServerConnections; // Only available on server.
  std::vector<std::pair<uint16_t, uint16_t>> mConnectionVersions;
  al::Socket mSocket; // Bootstrap socket for server, main socket for client.
  bool mVerbose{false};
};

class CommandServer : public CommandConnection {
public:
  bool start(uint16_t serverPort = 34450,
             const char *serverAddr = "localhost") override;
  void stop() override;

  /**
   * @brief Block until connectionCount connections are established
   * @return number of connections acquired during wait
   */
  uint16_t waitForConnections(uint16_t connectionCount, double timeout = 60.0);

  /**
   * @brief Wait for timeoutSecs for reply from all.
   * @param timeoutSecs
   * @return ping time for each connection. If time > timeout no ping received
   *
   * If timeout is 0, this function blocks until all replies are
   * received
   */
  std::vector<float> ping(double timeoutSecs = 1.0);

  size_t connectionCount();

  std::vector<std::pair<std::string, uint16_t>> connections();
  /**
   * @brief sendMessage
   * @param message
   * @param dst
   * @return
   *
   * if dst is nullptr, the message is sent to all registered sockets
   */
  bool sendMessage(uint8_t *message, size_t length, Socket *dst = nullptr,
                   ValueSource *src = nullptr) override;

protected:
private:
  std::unique_ptr<std::thread> mBootstrapServerThread;

  uint16_t mPortOffset = 12000;
};

class CommandClient : public CommandConnection {
public:
  bool start(uint16_t serverPort = 34450,
             const char *serverAddr = "localhost") override;

  bool sendMessage(uint8_t *message, size_t length, Socket *dst = nullptr,
                   al::ValueSource *src = nullptr) override;

  bool isConnected() { return mRunning && mSocket.opened(); }

protected:
  void clientHandlePing(Socket &client);

private:
  uint16_t mPortOffset = 12000;
  std::atomic<bool> mBusy{false};
};

} // namespace al

#endif // COMMANDCONNECTION_HPP
