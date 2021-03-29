#include "al/app/al_App.hpp"
#include "al/graphics/al_Font.hpp"

using namespace al;

struct MyApp : public App {
  FontRenderer fontLine1, fontLine2;

  size_t currentFrame = 0;

  void onCreate() {
    nav().pos(0, 0, 10);
    fontLine1.load(Font::defaultFont().c_str(), 64, 2048);
    fontLine1.write("Fixed text", 1.0);
    fontLine1.alignLeft();

    fontLine2.load(Font::defaultFont().c_str(), 48, 1024);
    fontLine2.alignLeft();
  }

  void onDraw(Graphics& g) {
    g.clear(0.2);
    // Because the font creates a texture in white, you must use tint() to set
    // its color.

    g.tint(1.0);
    fontLine1.renderAt(g, {0.0, 1.5, 0.0});

    g.pushMatrix();
    g.tint(0.5, 0.3, 0.9);
    g.translate(-2.0, 0.5, 0);
    std::string line = "Current frame: " + std::to_string(currentFrame);
    fontLine2.write(line.c_str(), 0.5);
    fontLine2.render(g);
    g.popMatrix();

    g.tint(1.0);
    // The static render() function from FontRender is very convenient, but
    // less performant.
    FontRenderer::render(g, "FPS:", Vec3d(0, -0.5, 0), 0.3);

    // Notice the difference between the rendering of this line and fontLine2.
    // The have the same "worldHeight" so they look the same size, but
    // because of their fontSize they look different.
    FontRenderer::render(g, std::to_string(graphicsDomain()->fps()).c_str(),
                         Vec3d(-.5, -1.0, 0), 0.5);
  }
};

int main() {
  MyApp app;
  app.start();
  return 0;
}
