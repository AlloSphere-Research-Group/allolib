
#include "al/app/al_App.hpp"
#include "al/app/al_GUIDomain.hpp"
#include "al/graphics/al_Shapes.hpp"

using namespace al;

class MyApp : public App {
public:
  Parameter x{"X", "", 0, -2.0, 2.0};
  Parameter y{"Y", "", 0, -2.0, 2.0};
  Parameter z{"Z", "", 0, -2.0, 2.0};

  ParameterColor color{"Color"};

  void onInit() override {
    auto GUIdomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto &gui = GUIdomain->newGUI();

    gui << x << y << z;
    gui << color;
  }

  void onCreate() override {
    nav() = Vec3d(0, 0, 2);
    addSphere(mMesh, 0.1);
    mMesh.primitive(Mesh::LINES);
  }

  void onDraw(Graphics &g) override {
    g.clear(0);

    g.pushMatrix();
    g.translate(x, y, z);
    g.color(color);
    g.draw(mMesh);
    g.popMatrix();
  }

private:
  Mesh mMesh;
};

int main() {
  MyApp app;
  app.dimensions(800, 600);
  app.title("GUI Domain example");
  app.start();
  return 0;
}
