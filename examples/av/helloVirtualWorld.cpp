/*
What is our "Hello world!" app?

An agent orbits around the origin emitting the audio line input. The camera
view can be switched between a freely navigable keyboard/mouse controlled mode
and a sphere follow mode.

Requirements:
2 channels of spatial sound
2 windows, one front view, one back view
stereographic rendering
*/

#include "al/core.hpp"
#include "al/core/sound/al_Ambisonics.hpp"
#include <cmath>
#include <iostream>

using namespace al;

#define AUDIO_BLOCK_SIZE 256

struct Agent : SoundSource, Nav {
  Agent() : oscPhase(0), oscEnv(1) {}

  virtual ~Agent() {}

  void onProcess(AudioIOData& io) {
    while (io()) {
      // float s = io.in(0);
      // float s = rnd::uniform(); // make noise, just to hear something
      float s = sin(oscPhase * M_2PI);
      // float s = al::rnd::uniformS();
      // s *= (oscEnv*=0.999);

      // if (oscEnv < 0.00001) {
      //   oscEnv = 1;
      //   oscPhase = 0;
      // }

      // float s = phase * 2 - 1;
      // oscPhase += 440. / io.framesPerSecond();
      oscPhase += 220. / io.framesPerSecond();
      if (oscPhase >= 1) oscPhase -= 1;
      writeSample(s * 0.1);
    }
  }

  virtual void onUpdateNav() {
    smooth(0.9);
    spin(M_2PI / 360, M_2PI / 397, 0);
    moveF(0.04);
    step();
    SoundSource::pose(*this);
  }

  virtual void onDraw(Graphics& g) {

    g.pushMatrix(Mat4f{Pose::matrix()});

    Mesh m{Mesh::TRIANGLES};
    float ds = 0.5;

    m.color(1, 1, 1);
    m.vertex(0, 0, ds * 2);
    m.color(1, 1, 0);
    m.vertex(ds / 2, 0, -ds);
    m.color(1, 0, 1);
    m.vertex(-ds / 2, 0, -ds);

    m.color(1, 1, 1);
    m.vertex(0, 0, ds * 2);
    m.color(0, 1, 1);
    m.vertex(0, ds / 2, -ds);
    m.color(1, 0, 1);
    m.vertex(-ds / 2, 0, -ds);

    m.color(1, 1, 1);
    m.vertex(0, 0, ds * 2);
    m.color(1, 1, 0);
    m.vertex(ds / 2, 0, -ds);
    m.color(0, 1, 1);
    m.vertex(0, ds / 2, -ds);

    g.meshColor();
    g.draw(m);
    g.popMatrix();
  }

  double oscPhase, oscEnv;
};

AudioScene scene(AUDIO_BLOCK_SIZE);
Listener* listener;
std::vector<Agent> agents(1);
Nav navMaster(Vec3d(0,0,50), 0.95);

struct MyWindow : App
{
  void onCreate() override {
    // set keyboard/mouse to control navMaster
    navControl().nav(navMaster);
  }

  void onDraw(Graphics& g) override {
    g.clear(0);
    // use controlled nav as view matrix
    g.viewMatrix(al::view_mat(navMaster));
    g.pushMatrix();
    g.scale(10);
    Mesh wb;
    addWireBox(wb);
    g.draw(wb);
    g.popMatrix();

    for (unsigned i = 0; i < agents.size(); ++i) {
      agents[i].onDraw(g);
    }
  }

  void onKeyDown(const Keyboard& k) override {
    // if (k.key() == Keyboard::TAB) {
    //  stereo.stereo(!stereo.stereo());
    // }
  }

  void onSound(AudioIOData& io) override {
    for (unsigned i = 0; i < agents.size(); ++i) {
      io.frame(0);
      agents[i].onUpdateNav();
      agents[i].onProcess(io);
    }
    navMaster.step(0.5);
    listener->pose(navMaster);
    io.frame(0);
    scene.render(io);
  }

};

int main(int argc, char* argv[]) {
  // Set speaker layout
  // const int numSpeakers = 2;
  Speaker speakers[] = {
      Speaker(0, 45, 0), Speaker(1, -45, 0),
  };
  SpeakerLayout speakerLayout;
  speakerLayout.addSpeaker(speakers[0]);
  speakerLayout.addSpeaker(speakers[1]);

  // Create spatializer
  AmbisonicsSpatializer* spat = new AmbisonicsSpatializer(speakerLayout, 2, 1);
  scene.usePerSampleProcessing(true);  // per sample processing is less
                                       // efficient than per buffer (default),
                                       // but provides higher quality Doppler

  // Create listener to render audio
  listener = scene.createListener(spat);

  // Now do some visuals
  auto us10 = [] { return 10 * rnd::uniformS(); };
  for (unsigned i = 0; i < agents.size(); ++i) {
    auto& ai = agents[i];
    // randomize position and orientation
    ai.Pose::pos(us10(), us10(), us10());
    ai.faceToward({us10(), us10(), us10()});
    scene.addSource(agents[i]);
  }

  MyWindow win;
  win.initAudio(44100, AUDIO_BLOCK_SIZE, 2, 0);
  win.start();

}
