#include "al/core.hpp"
#include <iostream>
#include <string>
#include <array>

using namespace al;
using namespace std;

class MyApp : public WindowApp {
public:
  ShaderProgram shader;
  VAOMesh mesh;

  void onCreate() {
    string const vert_source = R"(
      #version 330

      uniform mat4 m;

      layout (location = 0) in vec4 position;
      layout (location = 1) in vec4 color;
      layout (location = 2) in vec2 texcoord;

      out vec4 _color;

      void main() {
        gl_Position = m * position;
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

    mesh.primitive(TRIANGLES);
    mesh.vertex(-0.5, -0.5, 0);
    mesh.color(1.0, 0.0, 0.0);
    mesh.vertex(0.5, -0.5, 0);
    mesh.color(0.0, 1.0, 0.0);
    mesh.vertex(-0.5, 0.5, 0);
    mesh.color(0.0, 0.0, 1.0);
    mesh.vertex(-0.5, 0.5, 0);
    mesh.color(0.0, 0.0, 1.0);
    mesh.vertex(0.5, -0.5, 0);
    mesh.color(0.0, 1.0, 0.0);
    mesh.vertex(0.5, 0.5, 0);
    mesh.color(0.0, 1.0, 1.0);
    mesh.update();
  }

  void onDraw() {
    glViewport(0, 0, width(), height());
    GLfloat const clear_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, clear_color);

    float w = width();
    float h = height();
    Matrix4f mat = Matrix4f::rotate(sec(), 0, 0, 1);
    mat = Matrix4f::scaling(h / w, 1.0f, 1.0f) * mat;

    //Matrix4f proj = Matrix4f::perspective(60, w / h, 1, 100);
    
    shader.begin();
    shader.uniform("m", mat);
    mesh.draw();
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
      // mesh.color(1.0, 0.0, 0.0);
      mesh.vertex(0.5, -0.5, 0);
      // mesh.color(0.0, 1.0, 0.0);
      mesh.vertex(0, 0.5, 0);
      // mesh.color(0.0, 0.0, 1.0);
      mesh.update();
    }
    if (k.key() == 'z') {
      mesh.reset();
      mesh.vertex(-0.5, -0.5, 0);
      mesh.color(0.0, 0.0, 0.0);
      mesh.vertex(0.5, -0.5, 0);
      mesh.color(1.0, 0.0, 0.0);
      mesh.vertex(0.5, 0.5, 0);
      mesh.color(1.0, 1.0, 0.0);
      mesh.vertex(-0.5, 0.5, 0);
      mesh.color(0.0, 1.0, 0.0);
      mesh.index(0);
      mesh.index(1);
      mesh.index(2);
      mesh.index(0);
      mesh.index(2);
      mesh.index(3);
      mesh.update();
    }
  }
};

int main(int argc, char* argv[]) {
  MyApp app;
  app.start();
  return 0;
}