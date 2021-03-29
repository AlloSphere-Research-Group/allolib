/*
Allocore Example: Render Depth To Texture

Description:
This demonstrates how to render the depth buffer of a scene into a texture using
a frame buffer object (FBO).

Author:
Lance Putnam, 2015
*/

#include <iostream>
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Random.hpp"

using namespace al;
using namespace std;

struct MyApp : App {
  Mesh mesh;
  FBO fbo;           // Frame buffer object; this is our render target
  Texture texDepth;  // Texture to write depth buffer to

  void updateFBO(int w, int h) {
    // Configure texture on GPU
    texDepth.create2D(w / 4, h / 4, Texture::DEPTH_COMPONENT16,
                      Texture::DEPTH_COMPONENT, Texture::UBYTE);

    fbo.bind();
    // Attach the texture to the FBO
    fbo.attachTexture2D(texDepth, FBO::DEPTH_ATTACHMENT);
    fbo.unbind();
  }

  void onCreate() {
    addCylinder(mesh, 0.02);
    nav().pullBack(1.1);
    updateFBO(width(), height());
  }

  void onResize(int w, int h) { updateFBO(w, h); }

  void onDraw(Graphics& g) {
    gl::depthTesting(
        true);  // the depth buffer is not updated if the depth test is disabled

    // To render our scene to the FBO, we must first bind it
    fbo.bind();
    g.viewport(0, 0, texDepth.width(), texDepth.height());
    g.clearDepth();

    // Disable color rendering; we only want to write to the z-buffer
    g.colorMask(false);

    // Draw our scene
    g.draw(mesh);
    fbo.unbind();

    // Show depth buffer texture
    g.clear(1, 1, 1);
    g.viewport(0, 0, fbWidth(), fbHeight());
    g.colorMask(true);
    g.quadViewport(texDepth, -0.8, -0.8, 1.6, 1.6);
  }
};

int main() { MyApp().start(); }
