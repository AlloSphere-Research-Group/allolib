#include "al/core.hpp"
#include "al/util/ui/al_ParameterMIDI.hpp"

#include <cmath>

using namespace al;

Parameter Size("Size", "", 1.0, "", 0, 1.0);
Parameter Speed("Speed", "", 0.05, "", 0.01, 0.3);

ParameterMIDI parameterMIDI;

struct MyApp : App
{
	float x = 0;
	Mesh m;

	void onCreate() override {
		addSphere(m);
		nav().pos(0, 0, 4);
	}

	void onDraw(Graphics& g) override {
		g.clear(0);
		g.pushMatrix();
		g.translate(std::sin(x), 0, 0);
		g.scale(Size.get());
		g.draw(m);
		g.popMatrix();
		x += Speed.get();
		if (x >= M_2PI) {
			x -= M_2PI;
		}
	}

};

int main(int argc, char* argv[])
{
	parameterMIDI.connectControl(Size, 1, 1);
	parameterMIDI.connectControl(Speed, 10, 1);
	MyApp().start();
}

