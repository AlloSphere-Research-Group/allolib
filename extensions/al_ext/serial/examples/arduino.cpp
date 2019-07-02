/*
Here are the typical Arduino port addresses for each platform:
OSX		/dev/tty.usbmodem***
Linux	/dev/ttyACM0
Windows	COM6
*/

#include <string>
#include <iostream>
#include <cstdio>

#include "al/core.hpp"
#include "al/core/graphics/al_Shapes.hpp"
#include "al_ext/serial/al_Arduino.hpp"

using namespace al;

/*

The Arduino sketch for this example should be:
const int analogInPin = A0;
int sensorValue = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  sensorValue = analogRead(analogInPin);
  Serial.println(sensorValue);
  delay(0.1);
}

*/

class MyApp : public App {
public:

  Arduino arduino;

  float x = 0;

  void onInit() {
    // Query ports and print port list
    std::vector<serial::PortInfo> ports = serial::list_ports();

    for (auto port: ports) {
      std::cout << port.port << ":" << port.description << " -- HWID:" << port.hardware_id <<  std::endl;
    }

    // Hard code COM port for now. This should be auto detected
    std::string port("COM6");
    arduino.initialize(port);
  }

  void onAnimate(double dt) {
    // Read lines from Arduino serial port
    auto lines = arduino.getLines();
    for (auto line: lines) {
      std::cout << line << std::endl;
      // Convert to float and scale
      x = std::stof(line) / 1023.0f;
    }
  }

  void onDraw(Graphics &g) {
    Mesh m;
    addSphere(m);
    g.clear();
    // Use the data from the Arduino to position the sphere.
    g.translate(x, 0.0, -4);
    g.draw(m);
  }

  void onExit() {
    // Remember to cleanup or your app will crash on exit
    arduino.cleanup();
  }

};

int main(int argc, char **argv){
  MyApp().start();

  return 0;
}
