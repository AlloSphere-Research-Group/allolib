/*
Allocore Example: OSC Server

Description:
This is a simple OSC server that listens for packets with the address "/test"
and containing a string and int.

You should run the OSC client example AFTER running this program.

Author:
Lance Putnam, Oct. 2014
*/

#include <iostream>
#include <string>
#include "al/core.hpp"
using namespace al;

// App has osc::PacketHandler as base class
class MyApp : public App{
public:

	// can give params in ctor
	// osc::Recv server {16447, "", 0.05};

	// or open later with `open` interface (at onCreate in this example)
	osc::Recv server;

	// MyApp()
		/* Constructor args:
			port, client IP address ("" for any), timeout in seconds

		The port MUST match the client's port. The timeout specifies how often
		the server checks for new data on the on the port. You will	want to tune
		this to rate at which the client is sending packets.*/
	// :	server(16447, "", 0.05)
	void onCreate() {
		// Print out our IP address
		//std::cout << "SERVER: My IP is " << Socket::hostIP() << "\n";

		// port, address, timeout
		// "" as address for localhost
		server.open(16447, "", 0.05);

		// Register ourself (osc::PacketHandler) with the server so onMessage
		// gets called.
		server.handler(*this);

		// Start a thread to handle incoming packets
		server.start();

		// initWindow(Window::Dim(160), "OSC server");
	}

	// This gets called whenever we receive a packet
	void onMessage(osc::Message& m){

		// Check that the address and tags match what we expect
		if(m.addressPattern() == "/test" && m.typeTags() == "si"){

			// Extract the data out of the packet
			std::string str;
			int val;
			m >> str >> val;

			// Print out the extracted packet data
			std::cout << "SERVER: recv " << str << " " << val << "\n";
		}
	}

};


int main(){
	MyApp().start();
}
