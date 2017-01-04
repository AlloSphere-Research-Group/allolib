#include "al/core.hpp"
#include <iostream>
#include <string>
#include <array>

using namespace al;
using namespace std;

class MyApp : public WindowApp {
public:
  ShaderProgram shader;
  GLuint vao;
  GLuint position_buffer;
  BufferObject bufferObject;

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

    Mesh mesh = {GL_STATIC_DRAW};
    mesh.vertex(-0.5, -0.5, 0);
    mesh.vertex(0.5, -0.5, 0);
    mesh.vertex(0, 0.5, 0);
    for (auto const& v : mesh.vertices()) {
      cout << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << endl;
    }

    bufferObject.create();
    bufferObject.bind();
    bufferObject.data(4, mesh.vertices().size(), (float*)mesh.vertices().data());
    bufferObject.unbind();

    // glGenBuffers(1, &position_buffer);
    // glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
    // glBufferData(
    //   GL_ARRAY_BUFFER,
    //   sizeof(float) * 4 * mesh.vertices().size(),
    //   mesh.vertices().data(),
    //   GL_STATIC_DRAW
    // );
    // glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint index = 0;
    glEnableVertexAttribArray(index);
    // glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
    bufferObject.bind();
    glVertexAttribPointer(
      index,
      4, // size
      GL_FLOAT, // type
      GL_FALSE, // normalized
      0, // stride
      NULL // offset of the first component of the first generic vertex attribute
    );
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    bufferObject.unbind();
    glBindVertexArray(0);

  }

  void onDraw() {
    GLfloat const clear_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, clear_color);

    shader.begin();

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
    glBindVertexArray(0);

    shader.end();
  }

  void onExit() {
    // glDeleteBuffers(1, &position_buffer);
    glDeleteVertexArrays(1, &vao);
  }
};

int main(int argc, char* argv[]) {
  MyApp app;
  app.start();
  return 0;
}