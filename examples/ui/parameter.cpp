#include <iostream>

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/ui/al_Parameter.hpp"

using namespace al;
using namespace std;

struct MyApp : public App {
  // Parameter declaration
  // Access to these parameters is competely thread safe
  // constructor parameters are:
  // parameterName, Group, defaultValue, prefix, min, max
  //
  // parameterName, Group and prefix are used for display and to construct
  // the OSC path the parameter will listen to (see parameter_server example)
  //
  // The parameter will clamp values between min and max
  Parameter X{"X", "", 0.0, "", -1.0, 1.0};
  Parameter Y{"Y", "", 0.0, "", -1.0, 1.0};
  Mesh m;

  void onCreate() override {
    // Set the function to be called whenever the value of the "X"
    // parameter changes
    X.registerChangeCallback([&](float newX) {
      cout << "X has changed from " << X.get() << " to " << newX << endl;
    });

    // Have something to draw in a mesh
    addTorus(m);
    m.primitive(Mesh::LINE_STRIP);
    navControl().disable(); // Disable keyboard control of navigation
    std::cout << "Press 'w', 'x', 'a' or 'd'." << std::endl;
  }

  void onDraw(Graphics &g) override {
    g.clear();
    g.pushMatrix();
    // You can use the parameters as if they were regular floats (in most cases)
    g.translate(X, Y, -6.0); // Draw a mesh at X,Y
    g.draw(m);
    g.popMatrix();
  }

  bool onKeyDown(const Keyboard &k) override {
    // Use keyboard to control position of element
    switch (k.key()) {
    case 'w':
      Y = Y + 0.1;
      break;
    case 'x':
      Y = Y - 0.1;
      break;
    case 'a':
      X = X - 0.1;
      break;
    case 'd':
      X = X + 0.1;
      break;
    default:
      break;
    }
    return true;
  }
};

int main() {
  MyApp app;
  app.start();

  return 0;
}
