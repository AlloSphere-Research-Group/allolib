/*
Here are the typical Arduino port addresses for each platform:
OSX		/dev/tty.usbmodem***
Linux	/dev/ttyACM0
Windows	COM6
*/

#include <cstdio>
#include <iostream>
#include <string>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/io/al_Arduino.hpp"

using namespace al;

/*

The Arduino sketch for this example should be:

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print("0:");
  Serial.println(analogRead(A0));
  Serial.print("1:");
  Serial.println(analogRead(A1));
  delay(0.1);
}

*/

class MyApp : public App {
 public:
  Arduino arduino;

  float x = 0;
  float y = 0;

  void onInit() {
    // Query ports and print port list
    std::vector<serial::PortInfo> ports = serial::list_ports();

    // Show port information
    for (auto port : ports) {
      std::cout << port.port << ":" << port.description
                << " -- HWID:" << port.hardware_id << std::endl;
    }

    // Use last port, likely the right one on Windows
    auto lastPort = ports.back().port;
    // Initialize serial port
    arduino.init(lastPort, 9600);
  }

  void onAnimate(double dt) {
    // Read lines from Arduino serial port
    auto lines = arduino.getLines();
    for (auto line : lines) {
      auto separatorIndex = line.find(":");
      std::string id = line.substr(0, separatorIndex);
      std::string value = line.substr(separatorIndex + 1);
      std::cout << "id: " << id << " value: " << value << std::endl;
      if (id == "0") {
        // Convert to float and scale
        x = std::stof(value) / 1023.0f;
      } else if (id == "1") {
        y = std::stof(value) / 1023.0f;
      }
    }
  }

  void onDraw(Graphics &g) {
    Mesh m;
    addSphere(m);
    g.clear();
    // Use the data from the Arduino to position the sphere.
    g.translate(x, y, -4);
    g.draw(m);
  }

  void onExit() {
    // Remember to cleanup or your app will crash on exit
    arduino.cleanup();
  }
};

int main(int argc, char **argv) {
  MyApp().start();

  return 0;
}
