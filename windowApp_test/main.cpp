#include "al/core/base/al_GLEW.hpp"
#include "al/core/base/al_Window.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

namespace al {

// SINGLE window app
// makes window to be also an event handler
// by appending itself to the list of handlers
// also adds standard window controls, such as ctrl + q to quit
class WindowApp : public Window, public WindowEventHandler {
public:
  StandardWindowKeyControls stdControls;
  std::atomic<bool> should_quit;

  WindowApp() : should_quit(false) {
    append(stdControls);
    append(windowEventHandler());
  }
  virtual ~WindowApp() {}

  void start(
    const Window::Dim& dim = Window::Dim(800,600),
    const std::string title="",
    Window::DisplayMode mode = Window::DEFAULT_BUF
  ) {
    create(dim, title, mode);
    onCreate();
    while (!should_quit && created()) {
      onAnimate(0);
      onDraw();
      refresh();
      this_thread::sleep_for(chrono::milliseconds(100));
    }
    onExit();
  }

  void quit() {
    should_quit = true;
  }
  
  // user override these
  virtual void onCreate() {}
  virtual void onAnimate(double dt) {}
  virtual void onDraw() {}
  virtual void onExit() {}
  virtual void onKeydown(Keyboard const& k) {}
  virtual void onKeyUp(Keyboard const& k) {}
  virtual void onMouseDown(Mouse const& m) {}
  virtual void onMouseUp(Mouse const& m) {}
  virtual void onMouseDrag(Mouse const& m) {}
  virtual void onMouseMove(Mouse const& m) {}
  virtual void onResize(int w, int h) {}
  virtual void onVisibility(bool v) {}

  // call user event functions using WindowEventHandler class
  virtual bool keyDown(const Keyboard& k) override {
    onKeydown(k);
    return true;
  }
  virtual bool keyUp(const Keyboard& k) override {
    onKeyUp(k);
    return true;
  }
  virtual bool mouseDown(const Mouse& m) override {
    onMouseDown(m);
    return true;
  }
  virtual bool mouseDrag(const Mouse& m) override {
    onMouseDrag(m);
    return true;
  }
  virtual bool mouseMove(const Mouse& m) override {
    onMouseMove(m);
    return true;
  }
  virtual bool mouseUp(const Mouse& m) override {
    onMouseUp(m);
    return true;
  }
  virtual bool resize(int dw, int dh) override {
    onResize(dw, dh);
   return true; 
 }
  virtual bool visibility(bool v) override {
    onVisibility(v);
    return true; 
 }

};

}

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