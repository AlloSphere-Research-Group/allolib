

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_HtmlInterfaceServer.hpp"
#include "al/ui/al_Parameter.hpp"

using namespace al;

// Once program is running oint your browser to
// http://localhost:8080/Parameters.html

// This will look for interface.js alongside allolib
// Clone interface.js from git://github.com/charlieroberts/interface.js.git
std::string pathToInterfaceJs = "../../../../interface.js";

struct MyApp : public App {
  Mesh mesh;

  // The parameters
  Parameter X{"X", "Pos", 0, "", -2, 2};
  Parameter Y{"Y", "Pos", 0, "", -2, 2};
  Parameter Brightness{"Brightness", "Pos", 0.5, "", 0, 1};

  HtmlInterfaceServer htmlServer{pathToInterfaceJs};

  ControlGUI gui;

  void onCreate() override {
    addDodecahedron(mesh, 0.3);
    mesh.generateNormals();
    nav().pos() = Vec3d(0, 0, 6);

    // Connect parameters and presets to servers
    parameterServer() << X << Y << Brightness;

    // Then expose the server in the html server
    htmlServer << parameterServer();

    // Display value of X on stdout whenever it changes
    X.registerChangeCallback(
        [](float value) { std::cout << "X = " << value << std::endl; });

    navControl().disable();  // Disable so mouse drag doesn't control nav

    gui << X << Y << Brightness;
    gui.init();
  }

  virtual void onDraw(Graphics &g) override {
    g.clear();
    g.pushMatrix();
    g.color(Brightness.get());
    g.translate(X.get(), Y.get(), 0);
    g.draw(mesh);
    g.popMatrix();
    gui.draw(g);
  }
};

int main(int argc, char *argv[]) {
  MyApp().start();
  return 0;
}
