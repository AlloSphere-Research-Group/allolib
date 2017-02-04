#include "al/core.hpp"

#include <iostream>

#include <al/core/math/al_Matrix4.hpp>
#include <vector>

using namespace al;
using namespace std;

class MyApp : public WindowApp {
public:
  ShaderProgram shader;
  VAOMesh mesh;
  MatrixStack stack;
  Graphics g;
  
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

    g.setClearColor(0, 1, 1);
  }

  void onDraw() {
    g.viewport(0, 0, fbWidth(), fbHeight());
    g.clear();

    float w = width();
    float h = height();

    Matrix4f proj = Matrix4f::scaling(h / w, 1.0f, 1.0f);
    
    shader.begin();

    g.pushMatrix();
    g.rotate(sec(), 0, 0, 1);
    g.translate(-0.5, 0, 0);
    shader.uniform("m", proj * g.modelMatrix());
    mesh.draw();
    g.popMatrix();

    g.pushMatrix();
    stack.push();
    g.rotate(2 * sec(), 0, 0, 1);
    g.translate(0.5, 0, 0);
    shader.uniform("m", proj * g.modelMatrix());
    mesh.draw();
    g.popMatrix();

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