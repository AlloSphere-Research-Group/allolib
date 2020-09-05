#include "al/protocol/al_CommandConnection.hpp"

using namespace al;

enum {
  ASK_CLIENT_FOR_ORDER = CommandConnection::COMMAND_LAST_INTERNAL,
  TELL_SERVER_ORDER
};

class MyServer : public CommandServer {
public:
  void initiateConversation() {
    std::cout << "Server requesting order" << std::endl;
    for (auto listener : mServerConnections) {
      std::cout << "Sending command to " << listener->address() << ":"
                << listener->port() << std::endl;
      unsigned char message[8] = {0, 0};

      message[0] = ASK_CLIENT_FOR_ORDER;
      if (listener->send((const char *)message, 2) == 2) {
      } else {
        std::cerr << "ERROR sending command" << std::endl;
      }
    }
  }

  bool processIncomingMessage(Message &m, Socket *src) override {

    if (m.getByte() == TELL_SERVER_ORDER) {
      std::cout << "Got order from " << src->port() << " for " << m.getString()
                << std::endl;
      return true;
    }
    return false;
  }
};

class MyClient : public CommandClient {
public:
  bool processIncomingMessage(Message &m, Socket *src) override {

    if (m.getByte() == ASK_CLIENT_FOR_ORDER) {
      std::cout << src->port() << ":Let me think..." << std::endl;

      uint8_t message[7] = {TELL_SERVER_ORDER, 'w', 'a', 't', 'e', 'r', 0};

      if (!(mSocket.send((const char *)message, 7) == 7)) {
        std::cerr << "ERROR sending reply" << std::endl;
        return false;
      }
      return true;
    }
    return false;
  }
};

int main() {

  MyServer server;
  MyClient client;
  MyClient client2;

  if (!server.start(16010, "localhost")) {
    std::cerr << "ERROR starting command server" << std::endl;
  }

  if (!client.start(16010, "localhost")) {
    std::cerr << "ERROR starting command client" << std::endl;
  }

  if (!client2.start(16010, "localhost")) {
    std::cerr << "ERROR starting command client2" << std::endl;
  }

  // Wait until our client is connected
  server.waitForConnections(1);

  //  Ping from server
  server.ping();

  al_sleep(1.0);
  // Send message from server that generates a reply from the client
  server.initiateConversation();

  al_sleep(1.0);
  client.stop();
  client2.stop();
  server.stop();
  return 0;
}
