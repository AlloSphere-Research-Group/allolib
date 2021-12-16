/*
Allocore Example: Texture w/ Alpha Blending

Description:
Example of transparent blending of an image onto a scene using the image's
alpha channel. The alpha channel specifies the level of opaqueness, thus alpha=0
means totally transparent and alpha=1 means totally opaque.

For examples of more OpenGL blending modes, see

        http://www.andersriggelsen.dk/glblendfunc.php

Author:
Lance Putnam, Feb. 2015
*/
#include "al/app/al_App.hpp"
using namespace al;
using namespace std;

struct MyApp : public App {
  Mesh mesh;
  Texture tex;

  void onCreate() {
    // Generate a texture with an alpha channel for transparency
    tex.create2D(256, 256, Texture::RGBA8);
    int Nx = tex.width();
    int Ny = tex.height();

    // prepare vector for pixel data
    vector<Colori> pix; // byte color: rgba, [0:255]
    pix.resize(Nx * Ny);

    for (int j = 0; j < Ny; ++j) {
      float y = float(j) / (Ny - 1) * 2 - 1;
      for (int i = 0; i < Nx; ++i) {
        float x = float(i) / (Nx - 1) * 2 - 1;
        float px = x * (float)M_PI;
        float py = y * (float)M_PI;
        float z = cos(2 * px) * cos(5 * py) - cos(5 * px) * cos(2 * py);
        Color c = RGB(1);
        // Make alpha channel 1 where function is zero using Gaussian
        c.a = exp(-16 * z * z);
        pix[j * Nx + i] = c; // assignment converts float color to integer color
      }
    }
    tex.submit(pix.data());

    // Load an image having an alpha channel
    // Image img("myImage.png");
    // tex.allocate(img.array());

    mesh.primitive(Mesh::TRIANGLES);
    mesh.vertex(-1, -1, -1);
    mesh.color(RGB(1, 0, 0));
    mesh.vertex(1, -1, -1);
    mesh.color(RGB(1, 1, 0));
    mesh.vertex(0, 1, -1);
    mesh.color(RGB(0, 1, 1));

    nav().pos(0, 0, 4);
  }

  void onDraw(Graphics &g) {
    g.clear(0);

    // Draw all scene objects first
    g.meshColor(); // use mesh's color
    g.draw(mesh);

    // Here we activate transparent blending. This mixes source and
    // destination colors according to the source's alpha value.
    g.blending(true);
    g.blendTrans();

    // Render texture on a rectangle in world space ...
    g.quad(tex, -1, -1, 2, 2);

    // or render texture onto the viewport in normalized device coordinates
    // g.quadViewport(tex);

    // Turn off blending
    g.blending(false);
  }
};

int main() { MyApp().start(); }
