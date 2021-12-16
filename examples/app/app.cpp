/*
Allocore Example: Simple App

Description:
This example demonstrates how to use the App class to create a 3D space with
audio and graphics rendering callbacks.

The App class (and any subclass thereof) has some default keyboard and mouse
controls assigned automatically. These are:

    Keyboard		Action
    'w'				move forward
    'x'				move back
    'a'				move left
    'd'				move right
    'e'				move up
    'c'				move down
    'q'				roll counter-clockwise
    'z'				roll clockwise
    's'				stop moving
    '`'				reset navigation to (0,0,0) pointing along the z
axis left arrow		turn left right arrow		turn right up arrow
turn up down arrow		turn down

    escape			toggle fullscreen
    tab				toggle stereographic rendering
    ctrl-q			quit
    ctrl-h			hide window
    ctrl-m			minimize window

    Mouse
    drag left		turn left
    drag right		turn right
    drag up			turn up
    drag down		turn down

For the camera, the coordinate conventions are:

    -x is left
    +x is right
    -y is down
    +y is up
    -z is forward
    +z is backward

Author:
Lance Putnam, June 2011
*/

#include <iostream>
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"

// This example shows how to use the App class and its callbacks

using namespace al;

// We inherit from App to create our own custom application
struct MyApp : public App {
  Mesh mesh;
  double phase = 0;

  void onInit() override {
    std::cout << "onInit() - All domains have been initialized " << std::endl;
  }

  // This constructor is where we initialize the application
  void onCreate() override {
    std::cout << "onCreate() - Graphics context now available" << std::endl;

    // Add a unit sphere to the mesh
    addSphere(mesh);

    // Configure the camera lens
    lens().near(0.1).far(25).fovy(45);

    // Set navigation position and orientation
    nav().pos(0, 0, 4);
    nav().quat().fromAxisAngle(0. * M_2PI, 0, 1, 0);

    // This disables the default navigation keyboard/mouse controls;
    // leaving this enabled may cause some of the user input callbacks to
    // not trigger, such as onMouseDrag.
    // window().remove(navControl());

    // Initialize audio so that App::onSound is called
    // Arguments: sample rate (Hz), block size, output channels, input channels
  }

  // This is the audio callback
  void onSound(AudioIOData& io) override {
    // Things here occur at block rate...

    // This is the sample loop
    while (io()) {
      // float in = io.in(0);

      float out1 = 0;
      float out2 = 0;

      io.out(0) = out1;
      io.out(1) = out2;
    }
  }

  // This is the application's (graphical) model update.
  // This is called once for each frame of graphics. Typically, you will
  // update your application's geometry, physics, etc. here.
  void onAnimate(double dt) override {
    // The phase will ramp from 0 to 1 over 10 seconds. We will use it to
    // animate the sphere.
    double period = 10;
    phase += dt / period;
    if (phase >= 1.) phase -= 1.;
  }

  // This is the application's view update.
  // This is called one or more times per frame, for each window, viewport,
  // and eye (for stereoscopic). Typically, this is where you instruct the
  // GPU to render something.
  void onDraw(Graphics& g) override {
    // clear with a specified color (default is black)
    g.clear(HSV(0.5, 1, 0.5));

    // Note: we don't need to do all the normal graphics setup as this
    // is handled by the App
    // We can just draw our geometry immediately!

    g.polygonLine();  // wireframe mode
    g.pushMatrix();
    g.rotate(phase * 360, 0, 1, 0);
    g.color(1);
    g.draw(mesh);
    g.popMatrix();
  }

  // This is called whenever a key is pressed.
  bool onKeyDown(const Keyboard& k) override {
    // Use a switch to do something when a particular key is pressed
    switch (k.key()) {
      // For printable keys, we just use its character symbol:
      case '1':
        std::cout << "Pressed 1." << std::endl;
        break;
      case 'y':
        std::cout << "Pressed y." << std::endl;
        break;
      case 'n':
        std::cout << "Pressed n." << std::endl;
        break;
      case '.':
        std::cout << "Pressed period." << std::endl;
        break;
      case ' ':
        std::cout << "Pressed space bar." << std::endl;
        break;

        // For non-printable keys, we have to use the enums described in the
        // Keyboard class:
      case Keyboard::RETURN:
        std::cout << "Pressed return." << std::endl;
        break;
      case Keyboard::DELETE:
        std::cout << "Pressed delete." << std::endl;
        break;
      case Keyboard::F1:
        std::cout << "Pressed F1." << std::endl;
        break;
    }
    return true;
  }

  // This is called whenever a mouse button is pressed.
  bool onMouseDown(const Mouse& m) override {
    switch (m.button()) {
      case Mouse::LEFT:
        std::cout << "Pressed left mouse button." << std::endl;
        break;
      case Mouse::RIGHT:
        std::cout << "Pressed right mouse button." << std::endl;
        break;
      case Mouse::MIDDLE:
        std::cout << "Pressed middle mouse button." << std::endl;
        break;
      default:
        std::cout << "Pressed button " << m.button() << std::endl;
        break;
    }
    return true;
  }
  bool onMouseUp(const Mouse& m) override {
    switch (m.button()) {
      case Mouse::LEFT:
        std::cout << "Released left mouse button." << std::endl;
        break;
      case Mouse::RIGHT:
        std::cout << "Released right mouse button." << std::endl;
        break;
      case Mouse::MIDDLE:
        std::cout << "Released middle mouse button." << std::endl;
        break;
      default:
        std::cout << "Released button " << m.button() << std::endl;
        break;
    }
    return true;
  }

  // This is called whenever the mouse is dragged.
  bool onMouseDrag(const Mouse& m) override {
    // Get mouse coordinates, in pixels, relative to top-left corner of window
    int x = m.x();
    int y = m.y();
    std::cout << "Mouse dragged: " << x << "," << y << std::endl;
    return true;
  }

  // *****************************************************
  // NOTE: check the App class for more callback functions
};

int main() {
  // All we do in main is create the app and start it!
  MyApp app;
  app.dimensions(600, 400);
  app.title("Simple App");
  app.fps(40);
  app.audioDomain()->configure(44100, 128, 2, 1);  // rate, block, output, input
  app.start();
}
