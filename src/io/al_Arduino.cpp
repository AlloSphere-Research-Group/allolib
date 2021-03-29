#include <chrono>
#include <iostream>
#include <memory>
//#include <functional>

#include "al/io/al_Arduino.hpp"

#include "al/system/al_Time.hpp"

using namespace al;

bool Arduino::init(std::string port, unsigned long baud,
                         uint32_t granularity) {
  mGranularity = granularity;
  try {
    serialPort = std::make_unique<serial::Serial>(
        port, baud, serial::Timeout::simpleTimeout(granularity));
  } catch (const serial::IOException & /*e*/) {
    std::cerr << "ERROR: Can't open port " << port << std::endl;
    return false;
  }

  if (!serialPort->isOpen()) {
    std::cerr << "ERROR: Port " << port << " could not be opened" << std::endl;
    return false;
  }

  // Wait a bit for serial device to setup
  al::wait(0.5);
  // Flush buffers
  serialPort->flush();
  mRunning = true;
  mReaderThread =
      std::make_unique<std::thread>(std::bind(&Arduino::readFunction, this));

  return true;
}

void Arduino::cleanup() {
  if (mReaderThread) {
    mRunning = false;
    mReaderThread->join();
    mReaderThread = nullptr;
  }
  mRingBuffer.clear();
}

std::vector<std::string> Arduino::getLines() {
  std::vector<std::string> lines;
  const unsigned int bufferSize = 4096;
  char buffer[bufferSize];

  size_t count = mRingBuffer.read(buffer, bufferSize - 1);
  while (count > 0) {
    size_t start = 0;
    size_t nullPos = 0;
    while (nullPos <= count) {
      if (buffer[nullPos] == '\n' || buffer[nullPos] == '\r') {
        buffer[nullPos] = '\0';
        char *thisLine = buffer + start;
        mLineBuffer += std::string(thisLine);
        if (mLineBuffer.size() > 0) {
          lines.push_back(mLineBuffer);
          mLineBuffer.clear();
        }
        start = nullPos + 1;
      }
      nullPos++;
    }
    if (start < count) {
      char *thisLine = buffer + start;
      buffer[count] = '\0';
      mLineBuffer += std::string(thisLine);
    }
    count = mRingBuffer.read(buffer, bufferSize);
  }
  return lines;
}

void Arduino::readFunction() {
  const unsigned int bufferSize = 4096;
  uint8_t buffer[bufferSize];
  while (mRunning) {
    //        size_t bytes_wrote = my_serial.write(test_string);
    auto nextTime = std::chrono::high_resolution_clock::now() +
                    std::chrono::nanoseconds(mGranularity);
    try {
      size_t count = serialPort->read(buffer, bufferSize);
      mRingBuffer.write((char *)buffer, count);
      onInput(buffer, count);
      std::this_thread::sleep_until(nextTime);
    } catch (const serial::PortNotOpenedException & /*e*/) {
      mRunning = false;
      std::cerr << "ERROR: Port not opened exception. Closing serial device. "
                   "Call cleanup() before restarting."
                << std::endl;
    } catch (const serial::IOException & /*e*/) {
      mRunning = false;
      std::cerr << "ERROR: Port IO exception. Closing serial device. Call "
                   "cleanup() before restarting."
                << std::endl;
    }
  }
}
