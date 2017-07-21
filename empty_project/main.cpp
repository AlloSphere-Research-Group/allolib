#include "al/core.hpp"

#include <iostream>
#include <string>

using namespace al;
using namespace std;

class MyApp : public App {
public:
  Graphics g {*this};
  Viewpoint view;
  NavInputControl nav;
  ShaderProgram shader;
  VAOMesh mesh;

  void onCreate() {
    append(nav);
    nav.target(view);

    shader.compile(al_default_vert_shader(), al_default_frag_shader());

    view.pos(Vec3f(0, 0, 10)).faceToward(Vec3f(0, 0, 0), Vec3f(0, 1, 0));
    view.fovy(30).near(0.1).far(100);
    view.viewport(0, 0, fbWidth(), fbHeight());

    addIcosahedron(mesh);
    mesh.update();
  }

  void onAnimate(double dt) {
    nav.step();
  }

  void onDraw() {
    g.clearColor(0, 0, 0);
    g.shader(shader);
    g.camera(view);
    g.polygonMode(Graphics::LINE);
    g.draw(mesh);
  }

  void onKeyDown(Keyboard const& k) {

  }

  void onKeyUp(Keyboard const& k) {

  }

  void onSound(AudioIOData& io) {
    static double phase {0};
    double phaseIncrement = 440.0 / io.framesPerSecond();

    while(io()){
      phase += phaseIncrement;
      if(phase > 1) phase -= 1;
      float out = 0.1 * cos(phase * M_2PI);
      io.out(0) = out;
      io.out(1) = out;
    }
  }
};

int main() {
  MyApp app;
  app.fps(60);
  app.title("empty project");
  app.dimensions(960, 540);
  app.initAudio();
  app.start();
  return 0;
}