/*
Allocore Example: FBO

Description:
This demonstrates how to use an FBO
MipMaps:

FBOs won't generate the mip maps automatically
If texture filterMin is set to a MIPMAP option, then the texture will need to
have mipmaps generated manually (after the FBO is unbound), using
tex.generateMipmap();


Author:
Graham Wakefield, 2012
Keehong Youn, 2017

*/

#include <iostream>

#include "al/app/al_App.hpp"

using namespace al;
using namespace std;

int w = 256;
int h = 256;

EasyFBO fbo;
bool mipmap = true;

struct MyApp : public App {
  Mesh m{Mesh::TRIANGLES};
  Graphics gl;

  void onCreate() {
    fbo.init(w, h);

    m.vertex(-0.5f, -0.5f);
    m.vertex(0.5f, -0.5f);
    m.vertex(-0.5f, 0.5f);
  }

  void onDraw(Graphics& g) {
    // does not affect things drawn by gl
    g.translate(0.5, 0);

    // capture green-world to texture:
    gl.pushFramebuffer(fbo);
    gl.viewport(0, 0, w, h);
    gl.clear(0, 0.5, 0);
    gl.resetMatrixStack();
    gl.camera(Viewpoint::IDENTITY);
    gl.translate(0.2, 0.4);
    gl.color(1, 1, 0);
    gl.draw(m);
    gl.popFramebuffer();

    // show in blue-world:
    g.viewport(0, 0, fbWidth(), fbHeight());
    g.clear(0, 0, 0.5);
    g.quadViewport(fbo.tex(), -0.9, -0.9, 0.8, 0.8);  // x, y, w, h
  }
};

int main() {
  MyApp app;
  app.dimensions(800, 600);
  app.fps(10);
  app.start();
}
