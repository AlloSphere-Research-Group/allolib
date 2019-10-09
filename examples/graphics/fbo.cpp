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

#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"

#include <iostream>

using namespace al;
using namespace std;

struct MyApp : public App {
  int w = 256;
  int h = 256;

  Texture fbotex;
  RBO rbo;
  FBO fbo;
  bool mipmap = true;

  void onCreate() override {
    // both depth and color attachees must be valid on the GPU before use:
    rbo.create(w, h);
    fbotex.create2D(w, h);

    fbo.bind();
    fbo.attachTexture2D(fbotex);
    fbo.attachRBO(rbo);
    fbo.unbind();
    cout << "fbo status " << fbo.statusString() << endl;
  }

  void onDraw(Graphics& g) override {
    // capture green-world to texture:
    g.framebuffer(fbo);
    g.clear(0, 0.5, 0);
    g.viewport(0, 0, w, h);
    g.camera(Viewpoint::IDENTITY);

    Mesh m;
    m.primitive(Mesh::TRIANGLES);
    m.vertex(rnd::uniformS() * 0.5f, rnd::uniformS() * 0.5f);
    m.vertex(rnd::uniformS() * 0.5f, rnd::uniformS() * 0.5f);
    m.vertex(rnd::uniformS() * 0.5f, rnd::uniformS() * 0.5f);

    g.color(1, 1, 0);
    g.draw(m);

    // mipmaps can be generated after rendering to FBO
    if (mipmap) {
      fbotex.filter(
          Texture::LINEAR_MIPMAP_LINEAR);  // specify filtering for mipmap
      fbotex.generateMipmap();
    } else {
      fbotex.filter(Texture::NEAREST);
      fbotex.disableMipmap();
    }

    // show in blue-world:
    g.framebuffer(FBO::DEFAULT);
    g.clear(0, 0, 0.5);
    g.viewport(0, 0, fbWidth(), fbHeight());
    g.camera(Viewpoint::IDENTITY);
    g.quad(fbotex, -0.9, -0.9, 0.8, 0.8);  // x, y, w, h
    g.quad(fbotex, 0.1, -0.9, 0.4, 0.4);
    g.quad(fbotex, -0.9, 0.1, 0.2, 0.2);
    g.quad(fbotex, 0.1, 0.1, 0.1, 0.1);
  }

  bool onKeyDown(const Keyboard& k) override {
    if (k.key() == ' ') {
      mipmap = !mipmap;
      cout << "mipmap: " << mipmap << endl;
    }
    return true;
  }
};

int main() {
  MyApp app;
  app.dimensions(800, 600);
  app.fps(10);
  app.start();
}
