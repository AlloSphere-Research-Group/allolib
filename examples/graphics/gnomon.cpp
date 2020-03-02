#include "al/app/al_App.hpp"
#include "al/graphics/al_Font.hpp"
#include "al/ui/al_Gnomon.hpp"

using namespace al;

struct MyApp : public App {
  Gnomon gnomon;
  FontRenderer fontRender;

  void onCreate() { fontRender.load(Font::defaultFont().c_str(), 24, 1024); }

  void onDraw(Graphics& g) {
    g.clear(0.2f);

    // Draw at origin with labels
    gnomon.drawOrigin(g);
    gnomon.drawLabels(g, fontRender, pose());

    // draw at specific positions
    gnomon.drawAtPos(g, {-1.0, 0, -4});
    gnomon.drawAtPos(g, {0.5, 0.5, -4}, Pose(), 0.5);

    // draw in front of camera;
    gnomon.drawFloating(g, pose(), 0.1);
  }
};

int main() {
  MyApp app;
  app.start();
  return 0;
}
