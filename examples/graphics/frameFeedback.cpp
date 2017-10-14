/*
Allocore Example: Frame Feedback

Description:
This demonstrates how to create a feedback post-processing effect.
This is accomplished by copying the frame buffer into a texture after rendering
and then displaying the texture at the start of the next frame. Different
feedback effects can be accomplished by distorting the quad the texture is
rendered onto.

Author:
Lance Putnam, Nov. 2014
Keehong Youn, 2017
*/

#include "al/core.hpp"
#include <iostream>

using namespace al;

void testFunc (const char* msg, int ID) {
	std::cout << msg << ", id=" << ID << std::endl;
}

#if 0
#define AL_TEST(msg, ID) testFunc(msg, ID)
#else
#define AL_TEST(msg, ID) ((void)0)
#endif

class MyApp : public App {
public:

	Mesh shape;
	Texture texBlur;
	float angle = 0;

	void onCreate () override
	{
		// Create a colored square
		shape.primitive(Mesh::LINE_LOOP);
		const int N = 4;
		for(int i=0; i<N; ++i){
			float theta = float(i)/N * 2*M_PI;
			shape.vertex(cos(theta), sin(theta));
			shape.color(HSV(theta/2/M_PI));
		}

		texBlur.filter(GL_LINEAR);

		AL_TEST("test", 1);
		al::gl::error("test error", 30);
	}

	void onAnimate (double dt) override
	{
		angle += 2 * dt / 1000; // dt is in millis
		if(angle >= M_2PI) angle -= M_2PI;
	}

	void onDraw (AppGraphics& g) override
	{
		g.clear(0, 0);

		// 1. Match texture dimensions to viewport
		texBlur.resize(fbWidth(), fbHeight());

		// 2. Draw feedback texture. Try the different varieties!
		g.camera(Viewpoint::IDENTITY); // sets up [-1:1] x [-1:1] camera
		g.tint(0.98);
		// g.quad(texBlur, -1, -1, 2, 2);               // Plain (non-transformed) feedback
		g.quad(texBlur, -1.005, -1.005, 2.01, 2.01); // Outward feedback
		// g.quad(texBlur, -0.995, -0.995, 1.99, 1.99); // Inward feedback
		// g.quad(texBlur, -1.005, -1.00, 2.01, 2.0);   // Oblate feedback
		// g.quad(texBlur, -1.005, -0.995, 2.01, 1.99); // Squeeze feedback!
		g.tint(1); // set tint back to 1

		// 3. Do your drawing...
		g.camera(Viewpoint::UNIT_ORTHO); // ortho camera that fits [-1:1] x [-1:1]
		g.rotate(angle, 0,0,1);
		g.meshColor(); // use mesh's color array
		g.draw(shape);

		// 4. Copy current (read) frame buffer to texture
		texBlur.copyFrameBuffer();
	}

};

int main () {
	MyApp app;
	app.start();
}
