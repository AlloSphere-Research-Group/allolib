#include "al/core.hpp"
#include <iostream>
#include <string>
#include <array>

using namespace al;
using namespace std;

class MyApp : public WindowApp {
public:
  ShaderProgram shader;
  Texture texture;
  FBO fbo;
  Texture color;
  RBO depth;
  GLuint vao;

  void onCreate() {
    std::cout << "MyApp onCreate" << std::endl;

    string const vert_source = R"(
      #version 330

      layout (location = 0) in vec4 position;

      out vec4 _color;

      void main() {
        const vec4[] vertex_color = vec4[3](
          vec4(1.0, 0.0, 0.0, 1.0),
          vec4(0.0, 1.0, 0.0, 1.0),
          vec4(0.0, 0.0, 1.0, 1.0)
        );
        gl_Position = position;
        _color = vertex_color[gl_VertexID];
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

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLfloat positions[] = {
      -0.5, -0.5, 0, 1,
      0.5, -0.5, 0, 1,
      0, 0.5, 0, 1,
    };

    GLuint index = 0;
    glEnableVertexAttribArray(index);
    GLuint position_buffer;
    glGenBuffers(1, &position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(
      index,
      4, // size
      GL_FLOAT, // type
      GL_FALSE, // normalized
      0, // stride
      NULL // offset of the first component of the first generic vertex attribute
    );
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  void onDraw() {
    static int i = 0;

    GLfloat const clear_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, clear_color);

    shader.begin();

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
    glBindVertexArray(0);

    shader.end();

    if (i >= 600) {
      quit();
    }
    i += 1;
  }

};

int main(int argc, char* argv[]) {
  MyApp app;
  app.start();
  return 0;
}