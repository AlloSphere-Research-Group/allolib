/*
Allocore Example: Frame Feedback

Description:
This demonstrates how to create a feedback post-processing effect.
This is accomplished by copying the frame buffer into a texture after rendering
and then displaying the texture at the start of the next frame. Different
feedback effects can be accomplished by distorting the quad the texture is
rendered onto.

Author:
Lance Putnam, Nov. 2014
Keehong Youn, 2017
*/

#include <iostream>
#include "al/app/al_App.hpp"

using namespace al;

struct MyApp : public App {
  Mesh shape;
  Texture texBlur;
  float angle = 0;

  void onCreate() override {
    // Create a colored square
    shape.primitive(Mesh::LINE_LOOP);
    const int N = 4;
    for (int i = 0; i < N; ++i) {
      float theta = float(i) / N * 2 * M_PI;
      shape.vertex(cos(theta), sin(theta));
      shape.color(HSV(theta / 2 / M_PI));
    }

    texBlur.filter(Texture::LINEAR);
  }

  void onAnimate(double dt_sec) override {
    angle += dt_sec * 90;
    // angle += 90 * dt / 1000; // dt is in millis
    if (angle >= 360) angle -= 360;
  }

  void onDraw(Graphics& g) override {
    g.clear(0);

    // 1. Match texture dimensions to window
    texBlur.resize(fbWidth(), fbHeight());

    // 2. Draw feedback texture. Try the different varieties!
    g.tint(0.98);
    g.quadViewport(texBlur, -1.005, -1.005, 2.01, 2.01);  // Outward
    // g.quadViewport(texBlur, -0.995, -0.995, 1.99, 1.99); // Inward
    // g.quadViewport(texBlur, -1.005, -1.00, 2.01, 2.0);   // Oblate
    // g.quadViewport(texBlur, -1.005, -0.995, 2.01, 1.99); // Squeeze
    // g.quadViewport(texBlur, -1, -1, 2, 2);               // non-transformed
    g.tint(1);  // set tint back to 1

    // 3. Do your drawing...
    g.camera(Viewpoint::UNIT_ORTHO);  // ortho camera that fits [-1:1] x [-1:1]
    g.rotate(angle, 0, 0, 1);
    g.meshColor();  // use mesh's color array
    g.draw(shape);

    // 4. Copy current (read) frame buffer to texture
    texBlur.copyFrameBuffer();
  }
};

int main() {
  MyApp app;
  app.start();
}
