#include <atomic>
#include <vector>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"
#include "al/scene/al_DynamicScene.hpp"
#include "al/sound/al_Ambisonics.hpp"
#include "al/sound/al_Lbap.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/sound/al_StereoPanner.hpp"
#include "al/sound/al_Vbap.hpp"
#include "al/sphere/al_AlloSphereSpeakerLayout.hpp"
#include "al/ui/al_Parameter.hpp"

using namespace al;
using namespace std;

#define BLOCK_SIZE (512)

// This file demonstrates the different spatialization techniques
// doing block rendering in the Spatializer dervide classes.

struct MyApp : public App {
  Spatializer *spatializer{nullptr};

  Mesh mMarker;
  double speedMult = 0.04f;
  double mElapsedTime = 0.0;

  ParameterVec3 srcpos{"srcPos", "", {0.0, 0.0, 0.0}};
  atomic<float> *mPeaks{nullptr};

  Speakers speakerLayout;

  int speakerType = 0;
  int spatializerType = 0;
  unsigned long counter = 0; // overall sample counter

  void initSpeakers(int type = -1) {
    if (type < 0) {
      type = (speakerType + 1) % 3;
    }
    if (type == 0) {
      speakerLayout = AlloSphereSpeakerLayoutCompensated();
    } else if (type == 1) {
      speakerLayout = SpeakerRingLayout<8>(0, 0, 5);
    } else if (type == 2) {
      speakerLayout = StereoSpeakerLayout(0, 30, 5);
    }
    speakerType = type;
    if (mPeaks) {
      free(mPeaks);
    }
    mPeaks = new atomic<float>[speakerLayout.size()]; // Not being freed
                                                      // in this example
  }

  void initSpatializer(int type) {
    if (spatializer) {
      delete spatializer;
    }
    spatializerType = type;
    if (type == 1) {
      spatializer = new Lbap(speakerLayout);
    } else if (type == 2) {
      spatializer = new Vbap(speakerLayout, speakerType == 0);
    } else if (type == 3) {
      spatializer = new AmbisonicsSpatializer(speakerLayout, 3, 1, 1);
    } else if (type == 4) {
      spatializer = new AmbisonicsSpatializer(speakerLayout, 3, 2, 1);
    } else if (type == 5) {
      spatializer = new AmbisonicsSpatializer(speakerLayout, 3, 3, 1);
    } else if (type == 6) {
      spatializer = new StereoPanner(speakerLayout);
    }
    spatializer->compile();
  }

  void onInit() override {
    audioIO().channelsBus(1);
    addDodecahedron(mMarker);
    initSpeakers(0);
    initSpatializer(1);

    nav().pos(0, 3, 25);
    nav().faceToward({0, 0, 0});
  }

  void onAnimate(double dt) override {
    // Move source position
    mElapsedTime += dt;
    float tta = mElapsedTime * speedMult * 2.0f * M_PI;

    float x = 6.0f * cos(tta);
    float y = 5.0f * sin(2.8f * tta);
    float z = 6.0f * sin(tta);

    srcpos.set(Vec3d(x, y, z));
  }

  void onDraw(Graphics &g) override {
    g.clear(0);

    g.blending(true);
    g.blendTrans();
    // Draw the speakers
    for (size_t i = 0; i < speakerLayout.size(); ++i) {
      g.pushMatrix();
      float xyz[3];
      speakerLayout[i].posCart(xyz);
      g.translate(-xyz[1], xyz[2], -xyz[0]);
      float peak = mPeaks[i].load();
      g.scale(0.02f + fabs(peak) * 5);
      g.color(HSV(0.5f + (peak * 4)));
      g.polygonLine();
      g.draw(mMarker);
      g.popMatrix();
    }

    // Draw line to source position
    g.color(1);
    auto srcPosDraw = srcpos.get();
    Mesh lineMesh;
    lineMesh.vertex(0.0, 0.0, 0.0);
    lineMesh.vertex(srcPosDraw.x, 0.0, srcPosDraw.z);
    lineMesh.vertex(srcPosDraw);
    lineMesh.index(0);
    lineMesh.index(1);
    lineMesh.index(1);
    lineMesh.index(2);
    lineMesh.index(2);
    lineMesh.index(0);
    lineMesh.primitive(Mesh::LINES);
    g.draw(lineMesh);

    // Draw the source
    g.pushMatrix();
    g.polygonFill();
    g.scale(0.8f);
    g.color(0.4f, 0.4f, 0.4f, 0.5f);
    g.draw(mMarker);
    g.popMatrix();
  }

  virtual void onSound(AudioIOData &io) override {
    // Render signal to be panned
    while (io()) {
      float env = (22050 - (counter % 22050)) / 22050.0f;
      io.bus(0) = 0.5f * rnd::uniform() * env;
      ++counter;
    }
    //    // Spatialize
    spatializer->prepare(io);
    spatializer->renderBuffer(io, srcpos.get(), io.busBuffer(0),
                              io.framesPerBuffer());
    spatializer->finalize(io);

    // Now compute RMS to display the signal level for each speaker
    for (size_t speaker = 0; speaker < speakerLayout.size(); speaker++) {
      float rms = 0;
      for (unsigned int i = 0; i < io.framesPerBuffer(); i++) {
        unsigned int deviceChannel = speakerLayout[speaker].deviceChannel;
        float sample = io.out(deviceChannel, i);
        rms += sample * sample;
      }
      rms = sqrt(rms / io.framesPerBuffer());
      mPeaks[speaker].store(rms);
    }
  }

  bool onKeyDown(Keyboard const &k) override {
    audioIO().stop();
    if (k.key() == ' ') {
      initSpeakers();
      initSpatializer(spatializerType);
    }

    if (k.key() == '1') {
      initSpatializer(1);
    } else if (k.key() == '2') {
      initSpatializer(2);
    } else if (k.key() == '3') {
      initSpatializer(3);
    } else if (k.key() == '4') {
      initSpatializer(4);
    } else if (k.key() == '5') {
      initSpatializer(5);
    } else if (k.key() == '6') {
      initSpatializer(6);
    }
    audioIO().start();
    return true;
  }

  void onExit() override {
    if (mPeaks) {
      free(mPeaks);
    }
  }
};

int main() {
  MyApp app;

  // Set up Audio
  AudioDevice::printAll();
  app.configureAudio(44100, BLOCK_SIZE, 60, 0);
  // Use this for sphere
  //    app.configureAudio(AudioDevice("ECHO X5"),44100, BLOCK_SIZE, -1, -1);

  app.start();
  return 0;
}
