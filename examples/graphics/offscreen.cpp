#include "al/app/al_App.hpp"
#include "al/graphics/al_Graphics.hpp"

using namespace al;

struct offscreen : Graphics {
  EasyFBO fbo;

  void init(int w, int h) {
    fbo.init(w, h);
    Graphics::init();
  }

  void begin() {
    pushFramebuffer(fbo);
    pushViewport(0, 0, width(), height());
    resetMatrixStack();
  }

  void end() {
    popViewport();
    popFramebuffer();
  }

  int width() { return fbo.width(); }
  int height() { return fbo.height(); }
  Texture& tex() { return fbo.tex(); }
};

class MyApp : public App {
 public:
  Mesh m{Mesh::TRIANGLES};
  offscreen s;

  void onCreate() {
    s.init(256, 256);

    m.vertex(-0.5f, -0.5f);
    m.vertex(0.5f, -0.5f);
    m.vertex(-0.5f, 0.5f);
  }

  void onDraw(Graphics& g) {
    // capture green-world to offscreen:
    s.begin();
    s.clear(0, 0.5, 0);
    s.camera(Viewpoint::IDENTITY);
    s.color(1, 1, 0);
    s.draw(m);
    s.end();

    // show in blue-world:
    g.clear(0, 0, 0.5);
    g.quadViewport(s.tex(), -0.9, -0.9, 0.8, 0.8);
  }
};

int main() {
  MyApp app;
  app.dimensions(800, 600);
  app.start();
}
