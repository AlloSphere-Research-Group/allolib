#ifndef AL_ARDUINO_HPP
#define AL_ARDUINO_HPP

#include <memory>
#include <thread>

#include "serial/serial.h"

#include "al/core/types/al_SingleRWRingBuffer.hpp"

namespace al {


class Arduino {
public:
  bool initialize(std::string port = "", unsigned long baud = 9600);

  bool isOpen() { return mRunning && mReaderThread;}

  std::vector<std::string> getLines();


  void cleanup();

private:

  void readFunction();

  std::unique_ptr<serial::Serial> serialPort;
  SingleRWRingBuffer ringBuffer;

  bool mRunning {false};
  std::unique_ptr<std::thread> mReaderThread;

  std::string lineBuffer;

};

}

#endif // AL_SERIAL_HPP
