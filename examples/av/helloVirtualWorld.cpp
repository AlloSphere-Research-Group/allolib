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

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"
//#include "al/sound/al_Ambisonics.hpp"
#include "al/scene/al_DynamicScene.hpp"

#include <cmath>
#include <iostream>

using namespace al;

#define AUDIO_BLOCK_SIZE 256

// Create an agent,
struct Agent : PositionedVoice, Nav {
  float oscPhase{0}, oscFreq{220.0}, speed;

  void onProcess(AudioIOData& io) override {
    // Play a sine tone
    while (io()) {
      float s = std::sin(oscPhase * M_2PI);
      oscPhase += oscFreq / io.framesPerSecond();
      if (oscPhase >= 1) oscPhase -= 1;
      io.out(0) = s * 0.1f;
    }
  }

  void update(double dt) override {
    // Update internal smoothed nav
    smooth(0.9);
    spin(M_2PI / 360, M_2PI / 397, 0);
    moveF(speed);
    step();
    // Now update the current agent's position from the smoothed nav
    setPose(*this);
  }

  void onProcess(Graphics& g) override {
    // draw
    g.pushMatrix(/*Mat4f{Pose::matrix()}*/);

    Mesh m{Mesh::TRIANGLES};
    float ds = -0.5;

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
};

struct MyApp : App {
  DynamicScene scene;
  void onCreate() override {
    // Set initial pose
    nav() = {Vec3d(0, 0, 50), 0.95};

    auto us10 = [] { return 10.0 * rnd::uniformS(); };

    for (unsigned i = 0; i < 4; ++i) {
      auto* ai = scene.getVoice<Agent>();
      ai->oscFreq = 220.0f + (us10() * 220.0f);
      ai->speed = 0.1f + 0.1f * rnd::uniform();
      // randomize position and orientation
      // We can do it this way because the agent inherits from Nav
      ai->Pose::pos(us10(), us10(), us10());
      ai->faceToward({us10(), us10(), us10()});
      // Now insert into the scene
      scene.triggerOn(ai);
    }

    // Make distance changes more noticeable
    scene.distanceAttenuation().law(AttenuationLaw::ATTEN_INVERSE_SQUARE);
  }

  void onAnimate(double dt) override {
    // Uncomment this line to make listener pose be the viewing pose;
    //    scene.listenerPose(pose());
    scene.update(dt);
  }

  void onDraw(Graphics& g) override {
    g.clear(0);
    g.pushMatrix();
    g.scale(10);
    Mesh wb;
    addWireBox(wb);
    g.color(1);
    g.draw(wb);
    g.popMatrix();

    scene.render(g);
  }

  //  void onKeyDown(const Keyboard& k) override {
  //    // if (k.key() == Keyboard::TAB) {
  //    //  stereo.stereo(!stereo.stereo());
  //    // }
  //  }

  void onSound(AudioIOData& io) override { scene.render(io); }
};

int main() {
  MyApp win;
  win.configureAudio(44100, AUDIO_BLOCK_SIZE, 2, 0);
  win.start();

  return 0;
}
