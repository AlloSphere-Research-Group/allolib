
#include "catch.hpp"

#include "al/protocol/al_OSC.hpp"

using namespace al;

// #ifndef TRAVIS_BUILD

class Handler : public osc::PacketHandler {
public:
    Handler() {}
    virtual ~Handler() {}
	virtual void onMessage(osc::Message& m) override {
        m.print();
        address = m.addressPattern();
        m >> inString;
    }

    std::string address;
    std::string inString;
};

TEST_CASE( "OSC test" ) {
    Handler handler;

    osc::Recv server;
    server.open(10810, "localhost", 0.0);

    // Register ourself (osc::PacketHandler) with the server so onMessage
    // gets called.
    server.handler(handler);

    // Start a thread to handle incoming packets
    server.start();
    osc::Send(10810, "localhost").send("/hello", "world");

    al_sleep(0.1);

    REQUIRE(handler.address == "/hello");
    REQUIRE(handler.inString == "world");
}

TEST_CASE( "OSC multiple receivers" ) {
    Handler handler1;
    Handler handler2;

    osc::Recv server1;
    osc::Recv server2;
    server1.open(10810, "localhost", 0.0);
    server2.open(10820, "localhost", 0.0);

    server1.handler(handler1);
    server2.handler(handler2);

    // Start a thread to handle incoming packets
    server1.start();
    server2.start();

    osc::Send(10810, "localhost").send("/hello1", "world1");
    osc::Send(10820, "localhost").send("/hello2", "world2");

    al_sleep(0.1);

    REQUIRE(handler1.address == "/hello1");
    REQUIRE(handler1.inString == "world1");
    REQUIRE(handler2.address == "/hello2");
    REQUIRE(handler2.inString == "world2");
    

    osc::Send(10810, "localhost").send("/hello3", "world3");
    osc::Send(10820, "localhost").send("/hello4", "world4");

    al_sleep(0.5);

    REQUIRE(handler1.address == "/hello3");
    REQUIRE(handler1.inString == "world3");
    REQUIRE(handler2.address == "/hello4");
    REQUIRE(handler2.inString == "world4");
}

// #endif
