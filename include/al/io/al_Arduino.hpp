#ifndef AL_ARDUINO_HPP
#define AL_ARDUINO_HPP

#include <functional>
#include <thread>

#include "serial/serial.h"

#include "al/types/al_SingleRWRingBuffer.hpp"

namespace al {

/**
@defgroup IO I/O
*/
/**
@brief Arduino class
@ingroup IO
*/
class Arduino {
 public:
  Arduino(size_t ringBufferSize = 256) : mRingBuffer(ringBufferSize) {}

  /**
   * @brief initialize serial port and reader thread
   * @param port serial port name
   * @param baud baud rate
   * @param granularity the maximum time for port updates in nanoseconds
   * @return true if port was opened succesfully
   *
   * The granularity time determines the maximum time for port, but time might
   * less if the internal buffer fills
   */

  bool init(std::string port = "", unsigned long baud = 9600,
                  uint32_t granularity = 50);

  void cleanup();

  bool isOpen() { return mRunning && mReaderThread; }

  /**
   * @brief getLines received from serial port and remove them from input buffer
   * @return vector of lines that h
   */
  std::vector<std::string> getLines();

  std::function<void(uint8_t *, size_t)> onInput = [](uint8_t *, size_t) {};

 private:
  void readFunction();

  std::unique_ptr<serial::Serial> serialPort;
  SingleRWRingBuffer mRingBuffer;

  bool mRunning{false};
  std::unique_ptr<std::thread> mReaderThread;

  std::string mLineBuffer;
  uint32_t mGranularity;
};

}  // namespace al

#endif  // AL_SERIAL_HPP
