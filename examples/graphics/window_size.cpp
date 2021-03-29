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

// This example shows usage of the onResize callback for a window.

using namespace al;

struct MyApp : public App {
  void onCreate() override {
    // The window is not available before onCreate, so any uses of it must come
    // here or later on onDraw() or onAnimate(). For the same reason, the
    // initial resizing of the window due to the fullscreen call in the main
    // function will be missed, so if you need to process according to the size
    // at startup, make sure you add code here.

    defaultWindow().onResize = [&](int w, int h) {
      std::cout << "Resized to " << w << "," << h
                << " Framebuffer: " << fbWidth() << "," << fbHeight()
                << std::endl;
    };

    std::cout << "Initial Window size: " << width() << "," << height()
              << " Framebuffer: " << fbWidth() << "," << fbHeight()
              << std::endl;
  }

  void onDraw(Graphics& g) override { g.clear(0, 0.5, 0); }
};

int main() {
  MyApp app;

  //  app.dimensions(800, 600);
  app.fullScreen(true);

  app.start();
}
