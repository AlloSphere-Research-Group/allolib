/*
Allocore Example: Texture

Description:
This demonstrates how to create and display a texture.

Author:
Lance Putnam, Nov. 2013
*/

#include "al/app/al_App.hpp"
#include "al/math/al_Functions.hpp"
#include <iostream>
#include <vector>

using namespace al;
using namespace std;

struct MyApp : public App {
  Texture tex;
  Mesh mesh;

  void onCreate() {
    // Construct texture
    // Arguments: width, height, internal format, pixel format, pixel data type
    tex.create2D(64, 64, Texture::RGBA8, Texture::RGBA, Texture::UBYTE);

    // The default magnification filter is linear
    tex.filterMag(Texture::LINEAR);

    int stride = tex.numComponents();
    int Nx = tex.width();
    int Ny = tex.height();

    // Get a pointer to the (client-side) pixel buffer.
    // When we make a read access to the pixels, they are flagged as dirty
    // and get sent to the GPU the next time the texture is bound.
    vector<unsigned char> pixels;
    pixels.resize(stride * Nx * Ny);

    // Loop through the pixels to generate an image
    cout << Nx << ", " << Ny << endl;
    for (int j = 0; j < Ny; ++j) {
      float y = float(j) / (Ny - 1) * 2 - 1;
      for (int i = 0; i < Nx; ++i) {
        float x = float(i) / (Nx - 1) * 2 - 1;

        float m = 1 - al::clip(hypot(x, y));
        float a = al::wrap((float)atan2(y, x) / M_2PI);

        Color col = HSV(a, 1, m);

        int idx = j * Nx + i;
        pixels[idx * stride + 0] = col.r * 255.;
        pixels[idx * stride + 1] = col.g * 255.;
        pixels[idx * stride + 2] = col.b * 255.;
        pixels[idx * stride + 3] = col.a;
      }
    }

    tex.submit(pixels.data());

    // Generate the geometry onto which to display the texture
    mesh.primitive(Mesh::TRIANGLE_STRIP);
    mesh.vertex(-1, 1);
    mesh.vertex(-1, -1);
    mesh.vertex(1, 1);
    mesh.vertex(1, -1);

    // Add texture coordinates
    mesh.texCoord(0, 1);
    mesh.texCoord(0, 0);
    mesh.texCoord(1, 1);
    mesh.texCoord(1, 0);

    // The color acts as a multiplier on the texture color
    // mesh.color(RGB(1)); // white: shows texture as is
    // mesh.color(RGB(1,0,0)); // red: shows only red components

    nav().pullBack(4);
    // initWindow();
  }

  void onDraw(Graphics &g) {
    g.clear(0, 0, 0);

    // We must tell the GPU to use the texture when rendering primitives
    tex.bind();
    g.texture(); // Use texture for mesh coloring
    g.draw(mesh);
    tex.unbind();
  }
};

int main() { MyApp().start(); }
