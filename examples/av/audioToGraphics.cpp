/*
Allocore Example: Audio To Graphics

Description:
This example demonstrates how to visualize real-time audio. It uses a single
ring buffer to buffer audio samples between the audio and graphics threads. Two
sine waves are generated in the audio thread and drawn as a Lissajous curve in
the graphics thread.

Author:
Lance Putnam, 10/2012, putnam.lance@gmail.com
*/

#include "al/app/al_App.hpp"
#include "al/types/al_SingleRWRingBuffer.hpp"

using namespace al;

// This example shows how to use a Buffer class to pass data from the
// audio context to the graphics context.

class MyApp : public App {
 public:
  const size_t bufferSize = 8192;

  float bufferData[8192];
  double phase = 0;
  // Create ring buffer with size 2048, we will use this buffer to inter
  // leave the stereo samples from the audio callback
  SingleRWRingBuffer ringBuffer{bufferSize * sizeof(float)};
  Mesh curve;

  void onCreate() { nav().pos(0, 0, 4); }

  // Audio callback
  void onSound(AudioIOData& io) {
    // Set the base frequency to 55 Hz
    double freq = 55 / io.framesPerSecond();
    float out[2];

    while (io()) {
      // Update the oscillators' phase
      phase += freq;
      if (phase > 1) phase -= 1;

      // Generate two sine waves at the 5th and 4th harmonics
      out[0] = cos(5 * phase * 2 * M_PI);
      out[1] = sin(4 * phase * 2 * M_PI);

      // Write the waveforms to the ring buffer.
      ringBuffer.write((const char*)out, 2 * sizeof(float));

      // Send scaled waveforms to output...
      io.out(0) = out[0] * 0.2f;
      io.out(1) = out[1] * 0.2f;
    }
  }

  void onAnimate(double dt) {
    curve.primitive(Mesh::LINE_STRIP);
    curve.reset();

    size_t samplesRead =
        ringBuffer.read((char*)bufferData, bufferSize * sizeof(float));

    // Now we read samples from the buffer into the meash to be displayed
    for (size_t i = 0; i < samplesRead / sizeof(float); i = i + 2) {
      curve.vertex(bufferData[i], bufferData[i + 1]);
      // The redder the lines, the closer we are to a full ring buffer
      curve.color(HSV(0.5 * float(bufferSize) / (bufferSize - i)));
    }
  }

  void onDraw(Graphics& g) {
    g.clear(0);
    g.meshColor();
    g.draw(curve);
  }
};

int main() {
  MyApp app;
  app.configureAudio();  // init with out only
  app.start();
  return 0;
}
