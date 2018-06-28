#ifndef INCLUDE_AL_FLOWPARAMETERSERVERAPP_HPP
#define INCLUDE_AL_FLOWPARAMETERSERVERAPP_HPP

/*  Tim Wood, 2018, fishuyo@gmail.com
*/

#include "al/core/io/al_ControlNav.hpp"
#include "al/util/ui/al_Parameter.hpp"

#include <sstream>

namespace al {

class FlowParameterServerApp {
public:
	FlowParameterServer parameterServer;
	ParameterPose ppose{"pose"}; 
	Parameter mx{"mx"}, my{"my"}, mz{"mz"}, tx{"tx"}, ty{"ty"}, tz{"tz"};
	Parameter nearClip{"nearClip"}, farClip{"farClip"};
	Parameter eyeSeparation{"eyeSeparation"}, focalLength{"focalLength"};
	Parameter fovx{"fovx"}, fovy{"fovy"};

public:

	FlowParameterServerApp(){
		parameterServer.verbose();
		parameterServer << ppose << mx << my << mz << tx << ty << tz;
		parameterServer << nearClip << farClip << eyeSeparation << focalLength;
		parameterServer << fovx << fovy;
	};

	virtual const char* flowAddress(){ return "localhost"; };
	virtual uint16_t flowPort(){ return 12000; };
	virtual uint16_t recvPort(){ return 12001; };
    
    virtual std::string appName(){ return "defaultApp"; };
    
    virtual Nav& nav() = 0;
    virtual Lens& lens() = 0;

    void initFlowApp(){
		ppose.registerChangeCallback([&](Pose p){ nav().set(p); });
		
		mx.registerChangeCallback([&](float x){ nav().moveR(x); });
		my.registerChangeCallback([&](float x){ nav().moveU(x); });
		mz.registerChangeCallback([&](float x){ nav().moveF(x); });
		tx.registerChangeCallback([&](float x){ nav().spinR(x); });
		ty.registerChangeCallback([&](float x){ nav().spinU(x); });
		tz.registerChangeCallback([&](float x){ nav().spinF(x); });

		nearClip.registerChangeCallback([&](float x){ lens().near(x); });
		farClip.registerChangeCallback([&](float x){ lens().far(x); });
		eyeSeparation.registerChangeCallback([&](float x){ lens().eyeSep(x); });
		focalLength.registerChangeCallback([&](float x){ lens().focalLength(x); });
		fovx.registerChangeCallback([&](float x){ lens().fovx(x, 1); });
		fovy.registerChangeCallback([&](float x){ lens().fovy(x); });

    	parameterServer.listen(recvPort());
    	parameterServer.handshake(appName(), flowAddress(), flowPort());
    	parameterServer.sendMapping("lens", lensMapping());
    	parameterServer.sendMapping("joystickNav", joystickMapping());
    }

    std::string lensMapping(){ 
    	std::stringstream ss;
    	ss << "import ijs._\n\n";
    	ss << "val app = AppManager(\"" << appName() << "\")\n";
    	ss << "val io = Interface.create(\"lensParams\")\n";
    	ss << "io += Slider(\"nearClip\", x=0, y=0, w=0.1, h=0.9, min=0.000001, max=2.0)\n";
		ss << "io += Slider(\"farClip\", x=0.1, y=0, w=0.1, h=0.9, min=1.0, max=100.0)\n";
		ss << "io += Slider(\"eyeSeparation\", x=0.2, y=0, w=0.1, h=0.9, min=0.0, max=0.2)\n";
		ss << "io += Slider(\"focalLength\", x=0.3, y=0, w=0.1, h=0.9, min=1.0, max=100.0)\n";
		ss << "io += Slider(\"fovx\", x=0.4, y=0, w=0.1, h=0.9, min=30.0, max=100.0)\n";
		ss << "io += Slider(\"fovy\", x=0.5, y=0, w=0.1, h=0.9, min=30.0, max=100.0)\n";
		ss << "io.save(); io >> app\n";
    	return ss.str();
	}

    std::string joystickMapping(){
    	std::stringstream ss;
    	
		ss << "val joy = Device.joystick(0)\n";
		ss << "val app = AppManager(\"" << appName() <<"\")\n";
		ss << "val v = 0.15f   // move speed\n";
		ss << "val w = 0.01f  // turn speed\n";

		ss << "def dd(f:Float) = if(math.abs(f) < 0.06) 0 else f\n";

		ss << "joy.leftX.map(2 * _ - 1).map(dd).map(_ * v) >> app.sinks(\"mx\")\n";
		ss << "joy.leftY.map(2 * _ - 1).map(dd).map(_ * -v) >> app.sinks(\"mz\")\n";
		ss << "joy.rightX.map(2 * _ - 1).map(dd).map(_ * -w) >> app.sinks(\"ty\")\n";
		ss << "joy.rightY.map(2 * _ - 1).map(dd).map(_ * -w) >> app.sinks(\"tx\")\n";

		ss << "joy.upAnalog.map(_ * v) >> app.sinks(\"my\")\n";
		ss << "joy.downAnalog.map(_ * -v) >> app.sinks(\"my\")\n";
		ss << "joy.R2Analog.map(_ * v) >> app.sinks(\"my\")\n";
		ss << "joy.L2Analog.map(_ * -v) >> app.sinks(\"my\")\n";

		ss << "joy.R1Analog.map(_ * -w) >> app.sinks(\"tz\")\n";
		ss << "joy.L1Analog.map(_ * w) >> app.sinks(\"tz\")\n";
		// ss << "joy.select >> app.sinks("halt")"

    	return ss.str();
    }

};

} // al

#endif