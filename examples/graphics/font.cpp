#include "al/app/al_App.hpp"
#include "al/graphics/al_Font.hpp"

using namespace al;

struct MyApp : public App {
  Font font;
  Mesh mesh;

  void onCreate() {
    nav().pos(0, 0, 10);
    nav().setHome();
    font.load("data/VeraMono.ttf", 28, 1024);
    font.alignCenter();
    font.write(mesh, "hello font", 0.2f);
  }

  void onDraw(Graphics& g) {
    g.clear(0.5, 0.5, 0.5);
    g.blending(true);
    g.blendTrans();
    // g.camera(Viewpoint::IDENTITY); // Ortho [-1:1] x [-1:1]
    g.texture();
    font.tex.bind();
    g.draw(mesh);
    font.tex.unbind();
  }
};

int main() {
  MyApp app;
  app.start();
}
