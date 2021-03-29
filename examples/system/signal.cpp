#include <cstdlib>
#include <iostream>
#include "al/app/al_App.hpp"
#include "al/system/al_Signal.hpp"

/*

  Example file for showcasing how to utilize al_Signal.hpp

  SPACE key will put the program into a infinite while loop for testing purposes
  onExit() gets triggered on both normal exits and interrupt signals.

  Example shows sending an OSC message before the program quits.
  ** TODO: replace contents of onexit with signal safe instructions

  Kon Hyong Kim, 2019
  konhyong@gmail.com

*/

using namespace al;
using namespace std;

struct MyApp : App {
  Mesh mesh;
  double phase = 0;

  short port = 9010;
  const char* addr = "127.0.0.1";
  osc::Send client;

  void onCreate() override {
    client.open(port, addr);

    registerSigInt(this);  // *** need this line to register signal handler
  }

  void onExit() override {
    // although we use std::cout here for testing purposes,
    // avoid using functions that are not signal-safe.
    std::cout << "onExit!!!!" << std::endl;

    // example function call
    exitExtra();
  }

  void exitExtra() {
    std::cout << "doing extra stuff" << std::endl;
    // should use signal-safe functions
    client.send("/test", "handshakeoff", 1);
  }

  void onAnimate(double dt) override {}

  void onDraw(Graphics& g) override {}

  bool onKeyDown(Keyboard const& k) override {
    if (k.key() == ' ') {
      std::cout << "space" << std::endl;
      while (1)
        ;
    }
    return true;
  }

  void onMessage(osc::Message& m) override { m.print(); }
};

int main() {
  MyApp app;

  // registerSigInt(&app);  // alternate location for registerSigInt

  app.dimensions(600, 400);
  app.start();
}