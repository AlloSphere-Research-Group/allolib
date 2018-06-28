#ifndef INCLUDE_AL_DEVICESERVERAPP_HPP
#define INCLUDE_AL_DEVICESERVERAPP_HPP

/*  Tim Wood, 2018, fishuyo@gmail.com
*/

#include "al/core/protocol/al_OSC.hpp"
#include "al/core/io/al_ControlNav.hpp"


namespace al {

class DeviceServerApp {
public:

    class OSCHandler : public osc::PacketHandler {
    public:
        DeviceServerApp* app;
        OSCHandler(DeviceServerApp* a): app(a) {}
		void onMessage(osc::Message& m) override {
			app->onDeviceServerMessage(m);
		}
    };
    OSCHandler handler {this};

	virtual const char* address(){ return "ar01"; };
	virtual uint16_t outPort(){ return 12000; };
	virtual uint16_t inPort(){ return 12001; };

	osc::Recv oscRecv; // {inPort(), "", 0.05};
	osc::Send oscSend; // {outPort(), address(), 1024};
    
    virtual std::string name(){ return "defaultApp"; };
    virtual std::string config(){ return R"(
    {
		"io":{
			"name":"defaultApp",
			"sources":[],
			"sinks":[
				{"name":"mx", "type":"f"},
				{"name":"my", "type":"f"},
				{"name":"mz", "type":"f"},
				{"name":"tx", "type":"f"},
				{"name":"ty", "type":"f"},
				{"name":"tz", "type":"f"},
				{"name":"halt", "type":""},
			]
		},
		"defaultMappings":["joystickNav"]
	}
	)";};
    
    virtual Nav& nav() = 0;

    virtual void onDeviceServerMessage(osc::Message& m) {
		auto const addr = m.addressPattern();
		float x;
		// std::cout << addr << std::endl;
		if (addr == "/mx") {
			m >> x;
			nav().moveR(x);
		} else if (addr == "/my") {
			m >> x;
			nav().moveU(x);
		} else if (addr == "/mz") {
			m >> x;
			nav().moveF(x);
		} else if (addr == "/tx") {
			m >> x;
			nav().spinR(x);
		} else if (addr == "/ty") {
			m >> x;
			nav().spinU(x);
		} else if (addr == "/tz") {
			m >> x;
			nav().spinF(x);
		} else if (addr == "/home") {
			nav().home();
		} else if (addr == "/halt") {
			nav().halt();
		}
    }

    void initDeviceServer(){
    	oscRecv.open(inPort(), "", 0.05);
		oscSend.open(outPort(), address());
    	oscRecv.handler(handler);
		oscRecv.start();
		sendHandshake();
    }

    void sendHandshake(){
    	oscSend.send("/handshake", name(), inPort());
    }

};

} // al

#endif