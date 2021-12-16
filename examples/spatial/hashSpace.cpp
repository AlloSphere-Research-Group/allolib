/*
Allocore Example: Hashspace for nearest neighbor lookup.

Description:
Demonstration of neighbor and region lookup in Hashspace.
This example only uses a 2D slice, but it works just as well in 3D.

Author:
Graham Wakefield 2012
*/

#include <stdio.h>

#include <map>

#include "al/app/al_App.hpp"
#include "al/math/al_Functions.hpp"
#include "al/math/al_Random.hpp"
#include "al/spatial/al_HashSpace.hpp"

using namespace al;

struct World : public App {
  rnd::Random<> rng;

  // the space has 2^6 (64) voxels per side
  // (i.e., for each of the 3 sides) and up to 10000 objects:
  HashSpace space{6, 10000};
  unsigned maxradius{space.maxRadius()};

  // a query object to be re-used for finding neighbors
  // it will match up to 500 neighbors within a radius.
  HashSpace::Query qmany{500};

  // this is used for finding the nearest neighbor.
  // it will consider 6 matches and return the best.
  HashSpace::Query qnearest{6};
  double radius;

  void onCreate() override {
    radius = maxradius * 0.1;

    // initialize all objects with random position
    // just 2D for this demo
    for (unsigned id = 0; id < space.numObjects(); id++) {
      space.move(id, space.dim() * rng.uniform() * rng.uniform(),
                 space.dim() * rng.uniform() * rng.uniform(), 0);
    }
  }

  bool onMouseDrag(const Mouse& m) override {
    // drag to the right to increase radius, to the left to decrease
    double dx = m.dx() / (double)width();
    radius += dx * maxradius;
    return true;
  }

  void onDraw(Graphics& g) override {
    g.clear(0);

    double mx = space.dim() * mouse().x() / (double)width();
    double my = space.dim() * mouse().y() / (double)height();

    g.projMatrix(Matrix4f::ortho(0, space.dim(), space.dim(), 0, -1, 1));
    gl::depthTesting(false);

    // draw space grid:
    {
      Mesh m{Mesh::LINES};
      for (unsigned x = 0; x <= space.dim(); x++) {
        m.vertex(x, 0, 0);
        m.vertex(x, space.dim(), 0);
      }
      for (unsigned y = 0; y <= space.dim(); y++) {
        m.vertex(0, y, 0);
        m.vertex(space.dim(), y, 0);
      }
      g.color(0.2, 0.2, 0.2, 1);
      g.draw(m);
    }

    {
      Mesh m{Mesh::POINTS};
      for (unsigned id = 0; id < space.numObjects(); id++) {
        HashSpace::Object& o = space.object(id);

        // jiggle the objects around:
        double x = 0.5 + o.pos.x * 0.05;
        double y = 0.5 + o.pos.y * 0.05;
        double speed = 0.2 * sin(al_steady_time() + atan2(y, x));
        space.move(id, o.pos + Vec3d(speed * rng.uniformS(),
                                     speed * rng.uniformS(), 0.));

        m.vertex(o.pos);
      }
      g.pointSize(2);
      g.color(0.2, 0.5, 0.5);
      g.draw(m);
    }

    // draw active points:
    {
      Mesh m{Mesh::POINTS};
      qmany.clear();
      int results = qmany(space, Vec3d(mx, my, 0), radius);
      for (int i = 0; i < results; i += 1) {
        m.vertex(qmany[i]->pos);
      }
      g.color(1, 0.5, 0.2);
      g.draw(m);
    }

    // draw a circle around the mouse:
    {
      Mesh m{Mesh::LINE_LOOP};
      unsigned res = 30;
      for (unsigned i = 0; i <= res; i++) {
        double p = M_2PI * i / (double)res;
        m.vertex(mx + sin(p) * radius, my + cos(p) * radius, 0);
      }
      g.color(0.6, 0.1, 0.1, 1);
      g.draw(m);
    }

    // draw nearest neighbor links:
    {
      Mesh m{Mesh::LINES};
      double limit = space.maxRadius() * space.maxRadius() / 4.;
      for (unsigned id = 0; id < space.numObjects(); id++) {
        HashSpace::Object& o = space.object(id);
        HashSpace::Object* n = qnearest.nearest(space, &o);
        if (n) {
          Vec3d& v = n->pos;
          // don't draw if it is too long:
          if ((o.pos - v).magSqr() < limit) {
            m.vertex(o.pos);
            m.vertex(v);
          }
        }
      }
      g.color(0.2, 0.6, 0.2);
      g.draw(m);
    }
  }
};

int main() {
  World world;
  world.dimensions(600, 600);
  world.title("HashSpace collisions");
  world.start();
}
