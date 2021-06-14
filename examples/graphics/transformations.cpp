/*
Allocore Example: Matrix Transformations

Description:
This demonstrates how basic affine transformations, scaling, rotation, and
translation, can be applied when rendering meshes.

Author:
Lance Putnam, March 2015
*/

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
using namespace al;

struct MyApp : public App {
  Mesh mesh;

  void onCreate() {
    // Add wire-frame cube to mesh
    addWireBox(mesh);

    nav().pullBack(4);
  }

  void onDraw(Graphics &g) {
    g.clear();

    // The modelview matrix transforms each vertex before it is rendered.
    // First, we push a copy of the current modelview matrix onto a stack.
    // This allows us to save the matrix's current state and restore it at
    // a later point.
    g.pushMatrix();

    // Typically we scale, rotate, then translate. Note that we must apply
    // the transformations in the reverse order!
    g.translate(0, -0.5, 0);
    g.rotate(-45, 1, 0, 0);
    g.scale(0.5f, 0.3f, 0.1f);

    // Finally, draw mesh
    g.color(HSV(0.6f, 0.5f, 1));
    g.draw(mesh);

    // Pop the modified matrix off the stack to restore the original matrix.
    g.popMatrix();

    // We can draw the mesh as many times as we like with different
    // transformations. Here, we build a pyramid.
    for (int i = 1; i < 16; ++i) {
      float ds = 1. / 16;

      g.pushMatrix();

      g.translate(0, ds * (16 - i), 0);
      g.scale(ds * i, ds * 0.5f, ds * i);
      g.color(HSV(0.1f, 0.7f, 1));
      g.draw(mesh);

      g.popMatrix();
    }
  }
};

int main() { MyApp().start(); }
