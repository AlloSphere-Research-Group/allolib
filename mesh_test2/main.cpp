#include "al/core.hpp"
#include <iostream>
#include <string>
#include <array>

using namespace al;
using namespace std;

class MyApp : public WindowApp {
public:
  ShaderProgram shader;
  BufferObject position, color;
  VAO vao;
  VAOMesh mesh;

  void onCreate() {
    string const vert_source = R"(
      #version 330

      layout (location = 0) in vec4 position;
      layout (location = 1) in vec4 color;

      out vec4 _color;

      void main() {
        gl_Position = position;
        _color = color;
      }
    )";

    string const frag_source = R"(
      #version 330

      in vec4 _color;

      out vec4 frag_color;

      void main() {
        frag_color = _color;
      }
    )";

    shader.compile(vert_source, frag_source);

    mesh.init();
    mesh.primitive(TRIANGLES);
    mesh.vertex(-0.5, -0.5, 0);
    mesh.color(1.0, 0.0, 0.0);
    mesh.vertex(0.5, -0.5, 0);
    mesh.color(0.0, 1.0, 0.0);
    mesh.vertex(0, 0.5, 0);
    mesh.color(0.0, 0.0, 1.0);
    mesh.update();
  }

  void onDraw() {
    GLfloat const clear_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, clear_color);

    shader.begin();

    mesh.draw();
    // mesh.bind();
    // glDrawArrays(GL_TRIANGLES, 0, mesh.vertices().size());
    // mesh.unbind();

    shader.end();
  }

  void onKeyDown(Keyboard const& k) {
    if (k.key() == 'c') {
      mesh.reset();
      mesh.vertex(-0.5, -0.5, 0);
      mesh.color(0.0, 1.0, 0.0);
      mesh.vertex(0.5, -0.5, 0);
      mesh.color(0.0, 0.0, 1.0);
      mesh.vertex(0, 0.5, 0);
      mesh.color(1.0, 0.0, 0.0);

      mesh.vertex(0.0, 0.5, 0);
      mesh.color(0.0, 0.0, 1.0);
      mesh.vertex(0.5, -0.5, 0);
      mesh.color(0.0, 1.0, 0.0);
      mesh.vertex(0.5, 0.5, 0);
      mesh.color(1.0, 0.0, 0.0);
      mesh.update();
      return;
    }
    if (k.key() == 'v') {
      mesh.reset();
      mesh.vertex(-0.5, -0.5, 0);
      mesh.color(0.0, 1.0, 1.0);
      mesh.vertex(0.5, -0.5, 0);
      mesh.color(1.0, 0.0, 1.0);
      mesh.vertex(0, 0.5, 0);
      mesh.color(1.0, 1.0, 0.0);
      mesh.update();
      return;
    }
    if (k.key() == 'x') {
      mesh.reset();
      mesh.vertex(-0.5, -0.5, 0);
      mesh.color(1.0, 0.0, 0.0);
      mesh.vertex(0.5, -0.5, 0);
      mesh.color(0.0, 1.0, 0.0);
      mesh.vertex(0, 0.5, 0);
      mesh.color(0.0, 0.0, 1.0);
      mesh.update();
    }
  }

};

int main(int argc, char* argv[]) {
  MyApp app;
  app.start();
  return 0;
}