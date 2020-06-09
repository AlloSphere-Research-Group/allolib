#include "al/app/al_App.hpp"
#include "al/graphics/al_Image.hpp"
#include "al/graphics/al_Shapes.hpp"

using namespace al;
struct MyApp : public App {
  Mesh m;
  Texture tex;

  void onInit() override {
    addIcosahedron(m);

    tex.create2D(width(), height());
  }

  void onDraw(Graphics &g) override {
    g.clear(0.3);
    g.pushMatrix();
    g.translate(0, 0, -1.5);
    g.polygonFill();
    g.quad(tex, -1, -1, 2, 2);
    g.popMatrix();

    g.color(1);
    g.polygonLine();
    g.draw(m);
  }

  bool onKeyDown(const Keyboard &k) override {
    std::vector<unsigned char> mPixels;

    mPixels.resize(width() * height() * 3);
    unsigned char *pixs = mPixels.data();
    glReadPixels(1, 1, width(), height(), GL_RGB, GL_UNSIGNED_BYTE, pixs);
    Image::saveImage("capture.png", pixs, width(), height(), true);

    auto imageData = Image("capture.png");

    tex.submit(imageData.array().data(), GL_RGBA, GL_UNSIGNED_BYTE);

    return true;
  }
};

int main() {
  MyApp().start();

  return 0;
}
