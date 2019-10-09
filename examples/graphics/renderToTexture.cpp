/*
Allocore Example: Render To Texture

Description:
This demonstrates how to render a scene into a texture using a frame buffer
object (FBO). Rendering the scene to a texture requires an FBO with a texture
attached for the color buffer and a render buffer object (RBO) attached for the
depth buffer.

Author:
Lance Putnam, 2015
*/

#include <iostream>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"

using namespace al;
using namespace std;

struct MyApp : public App {
  Mesh mesh;
  FBO fbo;      // Frame buffer object; this is our render target
  RBO rbo;      // Render buffer object for depth buffer
  Texture tex;  // Texture for color buffer

  void updateFBO(int w, int h) {
    // Note: all attachments (textures, RBOs, etc.) to the FBO must have the
    // same width and height.

    // Configure texture on GPU
    tex.create2D(w, h);

    // Configure render buffer object on GPU
    rbo.resize(w, h);

    // Finally, attach color texture and depth RBO to FBO
    fbo.bind();
    fbo.attachTexture2D(tex);
    fbo.attachRBO(rbo);
    fbo.unbind();
  }

  void onCreate() {
    int Nv = addSphere(mesh);
    for (int i = 0; i < Nv; ++i) {
      mesh.color(HSV(float(i) / Nv * 0.3 + 0.2, 0.5, 1));
    }
    nav().pullBack(4);
    nav().faceToward(Vec3f(-2, -1, -1));
    updateFBO(width(), height());
  }

  void onResize(int w, int h) { updateFBO(w, h); }

  void onDraw(Graphics& g) {
    // To render our scene to the FBO, we must first bind it
    fbo.bind();
    // Clear FBO
    g.viewport(0, 0, tex.width(), tex.height());
    g.clear(0, 0, 0);

    // Render our scene as we normally would
    g.color(1, 0, 0);
    g.draw(mesh);

    // When done rendering our scene to the FBO, we must unbind it
    fbo.unbind();

    // To prove that this all worked, we render the FBO's color texture
    g.clear(1, 1, 1);
    g.viewport(0, 0, fbWidth(), fbHeight());
    g.quadViewport(tex, -0.9, -0.9, 1.8, 1.8);
  }
};

int main() { MyApp().start(); }
