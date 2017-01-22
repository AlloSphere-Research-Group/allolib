#include "al/core.hpp"

#include <iostream>

#include <al/core/math/al_Matrix4.hpp>
#include <vector>

namespace al {

class MatrixStack {
public:
  std::vector<Matrix4f> stack;

  MatrixStack() {
    // default constructor make identity matrix
    stack.emplace_back();
  }

  void push() {
    Matrix4f m = stack.back();
    stack.push_back(m);
  }

  void pop() {
    stack.pop_back();
  }

  void mult(Matrix4f const& m) {
    stack.back() = m * stack.back();
  }

  Matrix4f get() {
    return stack.back();
  }

  void setIdentity() {
    stack.back().setIdentity();
  }
};

}

using namespace al;
using namespace std;

class MyApp : public WindowApp {
public:
  ShaderProgram shader;
  VAOMesh mesh;
  MatrixStack stack;
  
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

    mesh.primitive(TRIANGLE_STRIP);
    mesh.vertex(-0.5, -0.5, 0);
    mesh.color(1.0, 0.0, 0.0);
    mesh.vertex(0.5, -0.5, 0);
    mesh.color(0.0, 1.0, 0.0);
    mesh.vertex(-0.5, 0.5, 0);
    mesh.color(0.0, 0.0, 1.0);
    mesh.vertex(0.5, 0.5, 0);
    mesh.color(0.0, 1.0, 1.0);
    mesh.update();
  }

  void onDraw() {
    glViewport(0, 0, fbWidth(), fbHeight());
    GLfloat const clear_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, clear_color);

    float w = width();
    float h = height();


    Matrix4f trans1 = Matrix4f::translation(-0.5f, 0.0f, 0.0f);
    Matrix4f trans2 = Matrix4f::translation(0.5f, 0.0f, 0.0f);
    Matrix4f rot1 = Matrix4f::rotate(sec(), 0, 0, 1);
    Matrix4f rot2 = Matrix4f::rotate(2 * sec(), 0, 0, 1);
    Matrix4f proj = Matrix4f::scaling(h / w, 1.0f, 1.0f);

    //Matrix4f proj = Matrix4f::perspective(60, w / h, 1, 100);
    
    shader.begin();

    stack.push();
    stack.mult(rot1);
    stack.mult(trans1);
    stack.mult(proj);
    shader.uniform("m", stack.get());
    mesh.draw();
    stack.pop();

    stack.push();
    stack.mult(rot2);
    stack.mult(trans2);
    stack.mult(proj);
    shader.uniform("m", stack.get());
    mesh.draw();
    stack.pop();

    shader.end();
  }
};

int main() {
  MyApp app;
  app.dimensions(640, 480);
  app.title("matrix test");
  app.start();
  return 0;
}