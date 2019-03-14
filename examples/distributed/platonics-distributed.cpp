
#include "al/core.hpp"
#include "al/core/app/al_DistributedApp.hpp"

using namespace al;

struct SharedState {
  double angle1, angle2;
};

struct DistributedExampleApp : DistributedApp<SharedState> {
  Mesh solids[5];

  void onCreate() {
    int Nv;

    Nv = addTetrahedron(solids[0]);
    for (int i = 0; i < Nv; ++i) {
      float f = float(i) / Nv;
      solids[0].color(HSV(f * 0.2 + 0.4, 1, 1));
    }

    Nv = addCube(solids[1]);
    for (int i = 0; i < Nv; ++i) {
      float f = float(i) / Nv;
      solids[1].color(HSV(f * 0.1 + 0.2, 1, 1));
    }

    Nv = addOctahedron(solids[2]);
    for (int i = 0; i < Nv; ++i) {
      float f = float(i) / Nv;
      solids[2].color(HSV(f * 0.1 + 0.1, 1, 1));
    }

    Nv = addDodecahedron(solids[3]);
    for (int i = 0; i < Nv; ++i) {
      float f = float(i) / Nv;
      solids[3].color(HSV(f * 0.1, 1, 1));
    }

    Nv = addIcosahedron(solids[4]);
    for (int i = 0; i < Nv; ++i) {
      float f = float(i) / Nv;
      solids[4].color(HSV(f * 0.1 + 0.7, 1, 1));
    }

    // Create face normals
    for (int i = 0; i < 5; ++i) {
      solids[i].decompress();
      solids[i].generateNormals();
    }

    nav().pos(0, 0, 15);

    if (hasRole(ROLE_RENDERER)) {
      load_perprojection_configuration();
      cursorHide(true);
    }
  }

  void simulate(double dt) {
    state().angle1 += 1. / 3;
    state().angle2 += M_PI / 3;
  }

  void onDraw(Graphics& g) {
    g.clear(0, 0, 0);

    g.depthTesting(true);
    g.lighting(true);

    float angPos = 2 * M_PI / 5;
    float R = 3;

    for (int i = 0; i < 5; ++i) {
      g.pushMatrix();
      g.translate(R * cos(i * angPos), R * sin(i * angPos), 0);
      g.rotate(state().angle1, 0, 1, 0);
      g.rotate(state().angle2, 1, 0, 0);
      g.color(0.5, 0.5, 0.5);
      g.polygonMode(Graphics::FILL);
      g.draw(solids[i]);
      g.scale(1.01);
      g.color(0);
      g.polygonMode(Graphics::LINE);
      g.draw(solids[i]);
      g.popMatrix();
    }
  }
};

int main() {
  DistributedExampleApp app;
  app.start();
}