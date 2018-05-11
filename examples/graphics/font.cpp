/*
Allocore Example: Rendering text with Font

Description:
This example shows how to load fonts and render text at various locations
in the window.

Author:
Lance Putnam, Sept. 2013
*/

#include "al/core.hpp"
#include "al/util/al_Font.hpp"
using namespace al;

class MyApp : public App {
 public:
  Font font1;
  Font font2;
  Font font3;

  void onCreate() {
    font1.load("data/VeraMoIt.ttf", 20);
    font2.load("data/VeraMoBd.ttf", 14);
    font3.load("data/VeraMono.ttf", 10);
  }

  void onDraw(Graphics& g) {
    g.clear(0.2);

    g.tint(1);
    g.quad(font1.texture(), -1, -1, 2, 2);

    // Get the viewport dimensions, in pixels, for positioning the text
    float W = fbWidth();
    float H = fbHeight();

    // Setup our matrices for 2D pixel space
    // g.projMatrix(Matrix4f::ortho2D(0, W, 0, H));
    // g.viewMatrix(Matrix4f::identity());

    // or there's oneliner
    g.camera(Viewpoint::ORTHO_FOR_2D);

    // Before rendering text, we must turn on blending
    g.blending(true);
    g.blendModeAdd();

    // Render text in the top-left corner
    g.pushMatrix();
    g.translate(8, H - (font1.size() + 8));
    g.tint(1, 1, 0, 1);
    font1.render(g, "Top-left text");
    g.popMatrix();

    // Render text in the bottom-left corner
    g.pushMatrix();
    g.translate(8, 8);
    g.tint(1, 0, 1, 1);
    font3.render(g, "Bottom-left text");
    g.popMatrix();

    // Render text centered on the screen
    g.pushMatrix();
    std::string str = "Centered text";
    // Note that dimensions must be integers to avoid blurred text
    g.translate(int(W / 2 - font2.width(str) / 2),
                int(H / 2 - font2.size() / 2));
    g.tint(0, 1, 1, 1);
    font2.render(g, str);
    g.popMatrix();

    // Turn off blending
    g.blending(false);

  }
};

int main() {
  MyApp app;
  app.dimensions(400, 200);
  app.start();
}
