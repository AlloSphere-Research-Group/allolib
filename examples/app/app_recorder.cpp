#include "al/app/al_App.hpp"
#include "al/app/al_AppRecorder.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"

#include "Gamma/Oscillator.h"

// This file shows how to use AppRecorder for 'offline' or non-real-time
// recording of an app's audio and video. The recorder will create a folder
// called 'capture_X' where all the frame stills and video is stored. If ffmpeg
// is available, it will build an mp4 video of these two together.

// Because offline recording takes over the app's clocks, audio is disabled and
// video is likely to run very slowly. Also because of this, you will need to
// set any sequencers to sync to graphics/animate or audio. The default of
// clocking to CPU will not work well as the graphics and audio clocks are
// likely to run slower than the CPU clock.

using namespace al;

struct MyApp : public App {

  // currentValue represents the shared state between audio and graphics
  // it is updated in onAnimate()
  double currentValue = 0.3;
  double timeAccum = 1.0;
  double inc = 0.0;

  // mesh for gtaphics
  Mesh m;

  // Oscillator for audio
  gam::Sine<> gen;

  // The AppRecorder object
  AppRecorder rec;

  void onCreate() { addCube(m); }

  void onAnimate(double dt) {
    // Create a change of target value every second.
    if (timeAccum >= 1.0) {
      timeAccum -= 1.0;
      inc = rnd::uniformS() / graphicsDomain()->fps();
    }
    timeAccum += dt;
    currentValue += inc;
  }

  void onDraw(Graphics &g) {
    g.clear(0);
    g.pushMatrix();
    g.translate(0, -1 + (currentValue * 2), -8);
    g.color(1);
    g.draw(m);
    g.popMatrix();
  }

  void onSound(AudioIOData &io) {
    // Update the frequency once per audio block
    // This is efficient but sound will change slightly
    // when buffer size or sampling rate changes
    gen.freq(300 * pow(2, currentValue));
    while (io()) {
      io.out(0) = gen() * 0.4;
    }
  }

  bool onKeyDown(const Keyboard &k) {
    if (k.key() == ' ') {
      // Press space bar to start recording
      rec.connectApp(this);
      // Maximum record time is 15 seconds, but we can stop it with the letter
      // 's'
      rec.startRecordingOffline(15.0);
    } else if (k.key() == 's') {
      rec.stopRecording();
    }
    return true;
  }
};

int main() {
  MyApp().start();

  return 0;
}
