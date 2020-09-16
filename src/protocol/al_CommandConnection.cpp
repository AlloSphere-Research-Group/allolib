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

  mConnectionsLock.lock();
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

  mConnectionsLock.unlock();

  return pingTimes;
}

size_t CommandServer::connectionCount() {
  mConnectionsLock.lock();
  size_t numConnections = mServerConnections.size();
  mConnectionsLock.unlock();
  return numConnections;
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
        uint8_t message[16];
        int bytesRecv = incomingConnectionSocket->recv((char *)message, 16);
        if (bytesRecv > 0 && bytesRecv < 9) {
          if (message[0] == HANDSHAKE) {
            uint16_t port;
            Convert::from_bytes((const uint8_t *)&message[1], port);

            if (mVerbose) {
              std::cout << "Handshake for "
                        << incomingConnectionSocket->address() << ":" << port
                        << std::endl;
            }

            message[0] = HANDSHAKE_ACK;
            message[1] = '\0';

            auto bytesSent =
                incomingConnectionSocket->send((const char *)message, 2);
            if (bytesSent != 2) {
              std::cerr << "ERROR sending handshake ack" << std::endl;
            }
            mConnectionsLock.lock();
            mServerConnections.emplace_back(incomingConnectionSocket);
            mConnectionsLock.unlock();

            mConnectionThreads.emplace_back(
                std::make_unique<std::thread>([&](al::Socket *client) {
                  while (mRunning) {
                    uint8_t commandMessage[1024];
                    size_t bytes = client->recv((char *)commandMessage, 1024);

                    if (bytes > 0 && bytes < 1025) {
                      if (commandMessage[0] == PONG) {
                        if (mVerbose) {
                          std::cout << "Got pong for " << client->address()
                                    << ":" << client->port() << std::endl;
                        }
                      } else if (commandMessage[0] == GOODBYE) {
                        std::cerr << "Goodbye not implemented" << std::endl;
                      } else if (commandMessage[0] == HANDSHAKE) {
                        std::cerr << "Unexpected handshake received"
                                  << std::endl;
                      } else {
                        Message message(commandMessage, bytes);
                        while (!message.empty()) {
                          message.print();
                          if (!processIncomingMessage(message, client)) {
                            std::cerr << "ERROR processing server message"
                                      << std::endl;
                          }
                        }
                      }
                    } else if (bytes != SIZE_MAX && bytes != 0) {
                      std::cerr << "ERROR unexpected command size" << bytes
                                << std::endl;
                      mRunning = false;
                    }
                  }

                  if (mVerbose) {
                    std::cout << "Client stopped " << std::endl;
                  }
                }, incomingConnectionSocket.get()));
          } else {
            std::cerr << "ERROR: Unrecognized server message "
                      << (int)message[0] << std::endl;
          }
        }
      }
    }
    //    incomingConnectionSocket->close();

    if (mVerbose) {
      std::cout << "Server quit" << std::endl;
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
  mConnectionsLock.lock();
  for (auto connectionSocket : mServerConnections) {
    connectionSocket->close();
  }
  mConnectionsLock.unlock();

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
      mConnectionsLock.lock();
      // TODO what should happen if there are disconnections instead of
      // connections while this runs?
      totalConnections = mServerConnections.size();
      mConnectionsLock.unlock();
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

bool CommandServer::sendMessage(std::vector<uint8_t> message, al::Socket *dst) {
  bool ret = true;
  if (message.size() == 0) {
    return false;
  }
  if (!dst) {
    for (auto listener : mServerConnections) {
      if (mVerbose) {
        std::cout << "Sending command to " << listener->address() << ":"
                  << listener->port() << std::endl;
      }
      ret &= listener->send((const char *)message.data(), message.size()) ==
             message.size();
    }
  } else {
    ret = dst->send((const char *)message.data(), message.size()) ==
          message.size();
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
    message[1] = b[0];
    message[2] = b[1];

    auto bytesSent = mSocket.send((const char *)message, 8);
    if (bytesSent != 8) {
      std::cerr << "ERROR sending handshake" << std::endl;
    }
    size_t bytesRecv = mSocket.recv((char *)message, 8);
    if (bytesRecv == 2 && message[0] == HANDSHAKE_ACK) {
      if (mVerbose) {
        std::cout << "Client got handshake ack" << std::endl;
      }
      mRunning = true;
    } else {
      return;
    }
    {
      std::unique_lock<std::mutex> lk(mutex);
      cv.notify_one();
    }

    while (mRunning) {
      if (!mSocket.opened()) {
        std::cerr << "ERROR seing dummy" << std::endl;
      }
      uint8_t commandMessage[8] = {0, 0, 0, 0, 0, 0, 0, 0};

      size_t bytes = mSocket.recv((char *)commandMessage, 8);

      if (bytes > 0 && bytes < 9) {
        if (commandMessage[0] == PING) {
          clientHandlePing(mSocket);
        } else {
          Message message(commandMessage, bytes);
          while (!message.empty()) {
            if (!processIncomingMessage(message, &mSocket)) {
              std::cerr << "ERROR: Unrecognized client message "
                        << (int)commandMessage[0] << " at " << mSocket.address()
                        << ":" << mSocket.port() << std::endl;
            }
          }
        }
      } else if (bytes != SIZE_MAX && bytes != 0) {
        std::cerr << "ERROR unexpected command size" << bytes << std::endl;
        mRunning = false;
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
    std::cout << "client got ping request" << std::endl;
  }
  char buffer[2] = {0, 0};
  buffer[0] = PONG;
  //  std::cout << "sending pong" << std::endl;
  int bytesSent = client.send((const char *)buffer, 2);
  if (bytesSent != 2) {
    std::cerr << "ERROR: sent bytes mismatch for pong" << std::endl;
  }
}
