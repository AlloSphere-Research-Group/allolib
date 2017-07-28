#include "al/core.hpp"

using namespace al;
using namespace std;

class MyApp : public EasyApp {
public:
  Mesh mesh;

  void onCreate() {
    addIcosahedron(mesh);
  }

  void onAnimate(double dt) {
    
  }

  void onDraw() {
    g.clearColor(0, 0, 0);
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
};

int main() {
  MyApp app;
  app.fps(60);
  app.title("empty project");
  app.dimensions(500, 500);
  app.initAudio();
  app.start();
  return 0;
}