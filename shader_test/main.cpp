#include "al/core.hpp"
#include <iostream>
#include <string>

using namespace al;
using namespace std;

class MyApp : public WindowApp {
public:
  ShaderProgram shader;
  GLuint vao;

  void onCreate() {
    std::cout << "MyApp onCreate" << std::endl;

    string const vert_source = R"(
      #version 330
      void main() {
        const vec4[] vertex = vec4[4](
          vec4(-0.5, -0.5, 0.0, 1.0),
          vec4(0.5, -0.5, 0.0, 1.0),
          vec4(-0.5, 0.5, 0.0, 1.0),
          vec4(0.5, 0.5, 0.0, 1.0)
        );
        gl_Position = vertex[gl_VertexID];
      }
    )";

    string const frag_source = R"(
      #version 330

      out vec4 frag_color;

      void main() {
        frag_color = vec4(0.0, 1.0, 1.0, 1.0);
      }
    )";

    auto compiled = shader.compile(vert_source, frag_source);
    cout << "shader compile successful? " << compiled << endl;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
  }

  void onDraw() {
    static int i = 0;
    GLfloat const color[] = {1.0f, 0.0f, 0.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, color);

    shader.begin();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    shader.end();

    if (i >= 6000) {
      // run for 600 frames and print average fps when done
      cout << (6000 / sec()) << endl;
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