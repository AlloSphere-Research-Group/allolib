/*
Allocore Example: OSC App

Description:
This shows how to use the onMessage callback in an app.

Launch an OSC message generator to see the messages this app receives

Author:
Andres Cabrera Sept 2019
*/

#include <iostream>
#include <string>
#include "al/app/al_App.hpp"
using namespace al;

struct MyApp : public App {
  // This gets called whenever we receive a packet
  void onMessage(osc::Message& m) override {
    m.print();
    // Check that the address and tags match what we expect
    if (m.addressPattern() == "/test" && m.typeTags() == "si") {
      // Extract the data out of the packet
      std::string str;
      int val;
      m >> str >> val;

      // Print out the extracted packet data
      std::cout << "SERVER: recv " << str << " " << val << std::endl;
    }
  }
};

int main() {
  MyApp().start();
  return 0;
}
