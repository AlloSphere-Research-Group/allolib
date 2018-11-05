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

// This will be picked up in the subdirectory because it has bee added in
// flags.cmake to the app_include_dirs variable

#include "drawing.hpp"

using namespace al;

class MyApp : public App {
public:

    void onCreate ()  override {
        drawing.init();
    }

    void onAnimate(double dt) override {
        drawing.tick(width(), height());
    }

    void onDraw (Graphics& g) override {
        // This unused variable will not report a warning because of the
        // compiler flags passed in app_compile_flags
        int unused = 42;
        g.clear(0, 0, 0);
        drawing.draw(g);
    }


    Drawing drawing;
};

int main () {
	MyApp app;
	app.start();
}
