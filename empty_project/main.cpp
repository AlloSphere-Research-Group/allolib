#include "al/core.hpp"

using namespace al;
using namespace std;

class MyApp : public App {
public:
  Mesh mesh;
  osc::Recv server {11117};

  void onCreate() {
    addIcosahedron(mesh);

	server.handler(*this);
	server.start();
  }

  void onAnimate(double dt) {
    
  }

  void onDraw(Graphics& g) {
    g.clear(0, 0, 0);
    g.color(1, 0, 0);
    g.polygonMode(Graphics::LINE);
    g.draw(mesh);
  }

  void onSound(AudioIOData& io) {
    static double phase {0};
    double phaseIncrement = 440.0 / io.framesPerSecond();

    while(io()){
      phase += phaseIncrement;
      if(phase > 1) phase -= 1;
      float out = 0.05 * cos(phase * M_2PI);
      io.out(0) = out;
      io.out(1) = out;
    }
  }

  void onMessage(osc::Message& m)
  {
  }
};

int main() {
  MyApp app;
  app.fps(60);
  app.title("empty project");
  app.dimensions(500, 500);
  app.initAudio();
  app.start();
}