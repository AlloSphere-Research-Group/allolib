/*
Allocore Example: 2D drawing

Description:
This demonstrates how to do 2D drawing by setting up an orthographic projection
matrix.

Author:
Lance Putnam, Feb. 2012
Keehong Youn, 2017
*/

#include "al/core.hpp"
using namespace al;

struct MyApp : public App {

	Mesh verts;
	Mesh verts2 {Mesh::LINES};

	void onCreate ()
	{
		verts.primitive(Mesh::LINE_STRIP);
		// Create a sine wave
		const int N = 128;
		for(int i=0; i<N; ++i){
			float f = float(i)/(N-1);
			verts.vertex(2*f-1, 0.5*sin(f*M_PI*2));
		}

	}

	void onAnimate(double dt)
	{
		const int N = 256;
		const float w = width();
		const float h = height();
		verts2.reset();
		for (int i = 0; i < N; i += 1) {
			float t = float(i) / (N - 1);
			verts2.vertex(w * t, h * (0.5 + 0.5 * cos(t * M_2PI)));
			verts2.color(t, 1 - t, 0);
		}
	}

	void onDraw (Graphics& g)
	{
		g.clear(0, 0, 0);
		g.camera(Viewpoint::IDENTITY);
		g.color(1, 1, 1);
		g.draw(verts);
		g.camera(Viewpoint::ORTHO_FOR_2D);
		g.meshColor();
		g.draw(verts2);
	}
};

int main () {
	MyApp app;
	app.start();
}
