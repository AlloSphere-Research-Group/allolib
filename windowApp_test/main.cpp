#include "al/core.hpp"
#include <iostream>

using namespace al;
using namespace std;

class MyApp : public WindowApp {
public:
  void onCreate() {
    std::cout << "MyApp onCreate" << std::endl;
  }

  void onDraw() {
    static int i = 0;
    GLfloat const color[] = {1.0f, 0.0f, 0.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, color);
    if (i >= 6000) {
      // run for 600 frames and print average fps when done
      std::cout << (6000 / sec()) << std::endl;
      quit();
    }
    i += 1;
  }

  void onKeyUp(Keyboard const& k) {
    std::cout << "onKeyUp" << char(k.key()) << std::endl;
  }

};

int main(int argc, char* argv[]) {
  MyApp app;
  app.fps(60);
  app.start();
  return 0;
}