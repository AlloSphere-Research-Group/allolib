#include "al/base/al_GLEW.hpp"
#include "al/base/al_Window.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace al;
using namespace std;

int main(int argc, char* argv[]) {
  Window win;
  StandardWindowKeyControls stdControls;

  win.create();
  win.prepend(stdControls);

  int i = 0;
  while (i < 100) {
    if (!win.created()) {
      // exit if window closed
      break;
    }

    // no `glClearColor` in GL3
    // glClearColor(1.0, 0.0, 0.0, 1.0);
    const GLfloat color[] = {1.0f, 0.0f, 0.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, color);

    cout << "frame " << i << endl;
    win.refresh(); // swap buffers and poll events
    i++;

    this_thread::sleep_for(chrono::milliseconds(100));
  }

  return 0;
}