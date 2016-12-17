#include "al/core.hpp"
#include <iostream>

using namespace al;
using namespace std;

class MyWindow : public WindowApp {
public:
  void onCreate() {
    std::cout << "MyWindow onCreate" << std::endl;
  }

  void onDraw() {
    static int i = 0;
    std::cout << "MyWindow onDraw " << i << std::endl;
    GLfloat const color[] = {1.0f, 0.0f, 0.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, color);
    if (i > 20) {
      quit();
    }
    i += 1;
  }

  void onKeyUp(Keyboard const& k) {
    std::cout << "onKeyUp" << char(k.key()) << std::endl;
  }

};

int main(int argc, char* argv[]) {
  MyWindow().start();
  return 0;
}