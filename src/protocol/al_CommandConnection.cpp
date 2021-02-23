#include "al/protocol/al_CommandConnection.hpp"

#include <algorithm>
#include <array>
#include <iostream>

namespace Convert {
auto to_bytes(std::uint16_t x) {
  std::array<std::uint8_t, 2> b;
  b[0] = x >> 8 * 0;
  b[1] = x >> 8 * 1;
  /*
b[2] = x >> 8 * 2;
b[3] = x >> 8 * 3;
b[4] = x >> 8 * 4;
b[5] = x >> 8 * 5;
b[6] = x >> 8 * 6;
b[7] = x >> 8 * 7;*/
  return b;
}

auto to_bytes(std::uint32_t x) {
  std::array<std::uint8_t, 4> b;
  b[0] = x >> 8 * 0;
  b[1] = x >> 8 * 1;
  b[2] = x >> 8 * 2;
  b[3] = x >> 8 * 3;
  /*b[4] = x >> 8 * 4;
b[5] = x >> 8 * 5;
b[6] = x >> 8 * 6;
b[7] = x >> 8 * 7;*/
  return b;
}

void from_bytes(const uint8_t *bytes, uint16_t &dest) {
  dest = (uint16_t(bytes[1]) << 8 * 1) | (uint16_t(bytes[0]) << 8 * 0);
}

void from_bytes(const uint8_t *bytes, uint32_t &dest) {
  dest = (uint16_t(bytes[3]) << 8 * 3) | (uint16_t(bytes[4]) << 8 * 2) |
         (uint16_t(bytes[1]) << 8 * 1) | (uint16_t(bytes[0]) << 8 * 0);
}

} // namespace Convert

using namespace al;

void CommandConnection::stop() {
  mRunning = false;
  mSocket.close();
  for (auto &connection : mConnectionThreads) {
    connection->join();
  }
  mState = BarrierState::NONE;
}

/// =====================================
///
std::vector<float> CommandServer::ping(double timeoutSecs) {
  std::vector<float> pingTimes;

  std::unique_lock<std::mutex> lk(mConnectionsLock);
  for (auto listener : mServerConnections) {
    if (mVerbose) {
      std::cout << "pinging " << listener->address() << ":" << listener->port()
                << std::endl;
    }
    auto startTime = al_steady_time();
    unsigned char message[2] = {0, 0};

    message[0] = PING;
    listener->send((const char *)message, 2);
    size_t bytes = 0;
    // FIXME need to check responses

    //    auto endTime = al_steady_time();
    //    if (bytes == 2 && message[0] == COMMAND_PONG) {
    //      std::cout << "Pong from " << listener->address() << ":"
    //                << listener->port() << " in " << (endTime - startTime) *
    //                1000.0
    //                << " ms" << std::endl;
    //    } else {
    //      std::cout << "No response from: " << listener->address() << ":"
    //                << listener->port() << std::endl;
    //      allResponded = false;
    //    }
  }

  return pingTimes;
}

size_t CommandServer::connectionCount() {
  std::unique_lock<std::mutex> lk(mConnectionsLock);
  size_t numConnections = mServerConnections.size();
  return numConnections;
}

std::vector<std::pair<std::string, uint16_t>> CommandServer::connections() {
  std::vector<std::pair<std::string, uint16_t>> cs;
  std::unique_lock<std::mutex> lk(mConnectionsLock);
  for (auto conn : mServerConnections) {
    cs.push_back({conn->address(), conn->port()});
  }
  return cs;
}

bool CommandServer::start(uint16_t serverPort, const char *serverAddr) {
  al_sec timeout = 0.5;
  if (!mSocket.open(serverPort, serverAddr, timeout, al::Socket::TCP)) {
    std::cerr << "ERROR opening port" << std::endl;
    return false;
  }
  if (!mSocket.bind()) {
    std::cerr << "ERROR on bind" << std::endl;
    return false;
  }

  if (!mSocket.listen()) {
    std::cerr << "ERROR on listen" << std::endl;
    return false;
  }

  mRunning = true;
  mBootstrapServerThread = std::make_unique<std::thread>([&]() {
    // Receive data
    if (mVerbose) {
      std::cout << "Server started" << std::endl;
    }

    while (mRunning) {
      std::shared_ptr<Socket> incomingConnectionSocket =
          std::make_shared<Socket>();
      if (mSocket.accept(*incomingConnectionSocket)) {
        if (mVerbose) {
          std::cout << "Got Connection Request "
                    << incomingConnectionSocket->address() << ":"
                    << incomingConnectionSocket->port() << std::endl;
        }
        uint8_t message[8192];

        int bytesRecv = incomingConnectionSocket->recv((char *)message, 8192);
        if (bytesRecv > 0 && bytesRecv <= 8192) {
          if (message[0] == HANDSHAKE) {
            uint16_t version = 0;
            uint16_t revision = 0;
            if (bytesRecv >= 4) {
              Convert::from_bytes((const uint8_t *)&message[1], version);
              Convert::from_bytes((const uint8_t *)&message[3], revision);
            }

            if (mVerbose) {
              std::cout << "Handshake for "
                        << incomingConnectionSocket->address() << ":"
                        << incomingConnectionSocket->port() << std::endl;
              std::cout << "Client reports protocol version " << version
                        << " revision " << revision << std::endl;
            }

            message[0] = HANDSHAKE_ACK;
            memcpy(message + 1, &mVersion, sizeof(uint16_t));
            memcpy(message + 1 + sizeof(uint16_t), &mRevision,
                   sizeof(uint16_t));

            auto bytesSent =
                incomingConnectionSocket->send((const char *)message, 5);
            if (bytesSent != 5) {
              std::cerr << "ERROR sending handshake ack" << std::endl;
            }
            {
              std::unique_lock<std::mutex> lk(mConnectionsLock);
              mServerConnections.emplace_back(incomingConnectionSocket);
              mConnectionVersions.emplace_back(
                  std::pair<uint16_t, uint16_t>{version, revision});
            }

            onConnection(incomingConnectionSocket.get());

            mConnectionThreads.emplace_back(std::make_unique<std::thread>(
                [&](std::shared_ptr<Socket> client) {
                  uint8_t commandMessage[8192];
                  size_t bufferSize = 0;
                  while (mRunning) {
                    size_t bytes = client->recv(
                        (char *)(commandMessage + bufferSize), 4096);

                    while (bytes > 0 && bytes < 4097) {
                      Message message(commandMessage, bytes);
                      if (mVerbose) {
                        std::cout << "Server received message from "
                                  << client->address() << ":" << client->port()
                                  << std::endl;
                      }
                      if (!processIncomingMessage(message, client.get())) {
                        std::cerr << __FILE__
                                  << " : Server unable to process message("
                                  << (int)commandMessage[0] << ") from client "
                                  << client->address() << ":" << client->port()
                                  << std::endl;
                      } else {
                        if (message.remainingBytes() > 0) {
                          memcpy(commandMessage, message.data(),
                                 message.remainingBytes());
                          bufferSize = message.remainingBytes();
                        } else {
                          bufferSize = 0;
                        }
                      }
                      bytes = client->recv(
                          (char *)(commandMessage + bufferSize), 4096);
                    }
                    if (bytes != SIZE_MAX && bytes != 0) {
                      std::cerr
                          << __FILE__
                          << " : Unexpected command size. Flushing buffer."
                          << bytes << std::endl;
                    }
                  }

                  if (mVerbose) {
                    std::cout << "Client stopped" << std::endl;
                  }
                },
                incomingConnectionSocket));
          } else {
            std::cerr << __FILE__ << ": Server unable to recognize message "
                      << (int)message[0] << std::endl;
          }
        }
      }
    }
    //    incomingConnectionSocket->close();

    if (mVerbose) {
      std::cout << "Server stopped" << std::endl;
    }
  });

  mState = CommandConnection::SERVER;
  return true;
}

void CommandServer::stop() {

  mRunning = false;
  mSocket.close();
  if (mBootstrapServerThread) {
    mBootstrapServerThread->join();
  }
  {
    std::unique_lock<std::mutex> lk(mConnectionsLock);
    for (auto connectionSocket : mServerConnections) {
      connectionSocket->close();
    }
  }

  for (auto &connection : mConnectionThreads) {
    connection->join();
  }
  mState = BarrierState::NONE;
}

uint16_t CommandServer::waitForConnections(uint16_t connectionCount,
                                           double timeout) {
  if (mState == BarrierState::SERVER) {
    double targetTime = al_steady_time() + timeout;
    double currentTime = al_steady_time();

    //    size_t existingConnections = mServerConnections.size();
    size_t existingConnections = 0;
    mConnectionsLock.lock();
    size_t totalConnections = mServerConnections.size();
    mConnectionsLock.unlock();
    while (targetTime >= currentTime) {
      // TODO what should happen if there are disconnections instead of
      // connections while this runs?
      {
        std::unique_lock<std::mutex> lk(mConnectionsLock);
        totalConnections = mServerConnections.size();
      }
      // FIXME this could allow more connections through than requested.
      // Should
      // the number be treated as a maximum?
      if (totalConnections - existingConnections < connectionCount) {
        al_sleep(0.3);
      } else {
        return totalConnections - existingConnections;
      }
      currentTime = al_steady_time();
    }
    return totalConnections - existingConnections;
  } else {
    // TODO make sure clients connect
  }
  return 0;
}

bool CommandServer::sendMessage(uint8_t *message, size_t length,
                                al::Socket *dst, al::ValueSource *src) {
  bool ret = true;
  if (length == 0) {
    return false;
  }

  if (!dst) {

    std::unique_lock<std::mutex> lk(mConnectionsLock);
    for (auto connection : mServerConnections) {
      if (!src || connection->address() != src->ipAddr ||
          connection->port() != src->port) {
        if (mVerbose) {
          std::cout << "Sending message to " << connection->address() << ":"
                    << connection->port() << std::endl;
        }
        ret &= connection->send((const char *)message, length) == length;
      }
    }

  } else {
    if (mVerbose) {
      std::cout << "Sending message to " << dst->address() << ":" << dst->port()
                << std::endl;
    }
    ret = dst->send((const char *)message, length) == length;
  }
  return ret;
}

// -----------------------------------------------------------------------

bool CommandClient::start(uint16_t serverPort, const char *serverAddr) {
  if (!mSocket.open(serverPort, serverAddr, 1.0, al::Socket::TCP)) {
    std::cerr << "Error opening bootstrap socket" << std::endl;
    return false;
  }
  std::condition_variable cv;
  std::mutex mutex;
  std::unique_lock<std::mutex> lk(mutex);
  mConnectionThreads.push_back(std::make_unique<std::thread>([&]() {
    // For a client connection, mSocket is connected to a server socket on
    // the other end.

    if (!mSocket.connect()) {
      std::cerr << "Error connecting bootstrap socket" << std::endl;
      return;
    }

    mState = CommandConnection::CLIENT;
    mRunning = true;
    unsigned char message[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    message[0] = HANDSHAKE;
    auto b = Convert::to_bytes(mSocket.port());

    memcpy(message + 1, &mVersion, sizeof(uint16_t));
    memcpy(message + 1 + sizeof(uint16_t), &mRevision, sizeof(uint16_t));

    // TODO provide functionality to validat connection versions
    auto bytesSent = mSocket.send((const char *)message, 5);
    if (bytesSent != 5) {
      std::cerr << "ERROR sending handshake" << std::endl;
    }
    size_t bytesRecv = mSocket.recv((char *)message, 5);
    if (bytesRecv == 5 && message[0] == HANDSHAKE_ACK) {
      uint16_t version = 0;
      uint16_t revision = 0;
      if (bytesRecv >= 4) {
        Convert::from_bytes((const uint8_t *)&message[1], version);
        Convert::from_bytes((const uint8_t *)&message[3], revision);
      }
      if (mVerbose) {
        std::cout << "Client got handshake ack from " << mSocket.address()
                  << ":" << mSocket.port() << std::endl;
        std::cout << "Server reports protocol version " << version
                  << " revision " << revision << std::endl;
      }
      mRunning = true;
    } else {
      return;
    }
    {
      std::unique_lock<std::mutex> lk(mutex);
      cv.notify_one();
    }
    uint8_t commandMessage[2048];
    size_t bufferSize = 0;

    onConnection(&mSocket);
    while (mRunning) {
      if (!mSocket.opened()) {
        std::cerr << "ERROR: Socket not open" << std::endl;
      }

      size_t bytes = mSocket.recv((char *)(commandMessage + bufferSize), 1024);
      if (bytes > 0 && bytes < 1025) {
        bufferSize += bytes;
        if (commandMessage[0] == PING) {
          clientHandlePing(mSocket);
        } else {
          Message message(commandMessage, bytes);
          if (mVerbose) {
            std::cout << "Client received message from " << mSocket.address()
                      << ":" << mSocket.port() << std::endl;
          }
          if (!processIncomingMessage(message, &mSocket)) {
            std::cerr << __FILE__ << " : Client unable to process message("
                      << (int)commandMessage[0] << ") from "
                      << mSocket.address() << ":" << mSocket.port()
                      << std::endl;

          } else {
            if (message.remainingBytes() > 0) {
              memcpy(commandMessage, message.data(), message.remainingBytes());
              bufferSize = message.remainingBytes();
            } else {
              bufferSize = 0;
            }
          }
        }
      } else if (bytes != SIZE_MAX && bytes != 0) {
        std::cerr << "ERROR: Network buffer overrun. " << bytes << std::endl;
      }
    }
    //        connectionSocket.close();
    if (mVerbose) {
      std::cout << "Client stopped " << std::endl;
    }
  }));

  cv.wait(lk);
  return true;
}

void CommandClient::clientHandlePing(Socket &client) {
  if (mVerbose) {
    std::cout << "Client got ping request" << std::endl;
  }
  char buffer[2] = {0, 0};
  buffer[0] = PONG;
  //  std::cout << "sending pong" << std::endl;
  int bytesSent = client.send((const char *)buffer, 2);
  if (bytesSent != 2) {
    std::cerr << "ERROR: sent bytes mismatch for pong" << std::endl;
  }
}

bool CommandClient::sendMessage(uint8_t *message, size_t length, Socket *dst,
                                al::ValueSource *src) {
  bool ret = true;
  if (length == 0) {
    return false;
  }

  if (!dst) {
    if (!src || mSocket.address() != src->ipAddr ||
        mSocket.port() != src->port) {
      if (mVerbose) {
        std::cout << "Sending message to " << mSocket.address() << ":"
                  << mSocket.port() << std::endl;
      }
      ret = mSocket.send((const char *)message, length) == length;
    }
  } else {
    if (mSocket.address() != dst->address() || mSocket.port() != dst->port()) {
      if (mVerbose) {
        std::cout << "Sending message to " << dst->address() << ":"
                  << dst->port() << std::endl;
      }
      ret = dst->send((const char *)message, length) == length;
    }
  }
  return ret;
}
